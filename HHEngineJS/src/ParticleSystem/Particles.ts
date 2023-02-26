// @ts-nocheck

import {huahuoEngine} from "../EngineAPI";
import {AbstractComponent, Component, PropertyValue} from "../Components/AbstractComponent";
import {PropertyCategory} from "../Components/PropertySheetBuilder";
import {GlobalConfig} from "../GlobalConfig";

const MAX_PARTICLE_COUNT = 1000

function Vector3ToArray(vec: Vector3) {
    return [vec.x, vec.y, vec.z]
}

function ColorToArray(color) {
    return [color.red, color.green, color.blue, color.alpha]
}

// ParticleSystem is not compatible with any shape. It should be used in ParticleSystemRenderer only as a subcomponent.
@Component({compatibleShapes: []})
class Particles extends AbstractComponent {
    _particles
    _currentActiveParticleNumber;

    @PropertyValue(PropertyCategory.interpolateFloat, 10.0)
    particleSize

    @PropertyValue(PropertyCategory.interpolateVector3, {x: 100.0, y: 100.0, z: 100.0})
    initMaxVelocity

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 3.0, y: 3.0}) // Unit is seconds.
    lifeSpanRange

    @PropertyValue(PropertyCategory.interpolateFloat, 100, {max: MAX_PARTICLE_COUNT})
    activeParticleCount

    @PropertyValue(PropertyCategory.interpolateFloat, 100)
    particleMass

    @PropertyValue(PropertyCategory.interpolateColor, {random: true})
    particleColor

    @PropertyValue(PropertyCategory.interpolateVector3, {x: 0.0, y: -98, z: 0.0})
    gravity

    maxNumbers = MAX_PARTICLE_COUNT // Preload MAX_PARTICLE_COUNT particles.

    lastUpdatedFrameId = -1;


    // This buffer is only used for IDE. During runtime, no need to have this one.
    frameDataBuffers

    constructor(rawObj?) {
        super(rawObj)

        let particleType = huahuoEngine.ti.types.struct({
            velocity: huahuoEngine.ti.types.vector(huahuoEngine.ti.f32, 3),
            position: huahuoEngine.ti.types.vector(huahuoEngine.ti.f32, 3),
            status: huahuoEngine.ti.i32,
            bornFrameId: huahuoEngine.ti.i32,
            lastUpdatedFrameId: huahuoEngine.ti.i32,
            life: huahuoEngine.ti.i32
        })

        this._particles = huahuoEngine.ti.field(particleType, [this.maxNumbers])


        this._currentActiveParticleNumber = huahuoEngine.ti.field(ti.f32, [1])

        huahuoEngine.ti.addToKernelScope({
            particles: this._particles,
            currentActiveParticleNumber: this._currentActiveParticleNumber,
            maxNumbers: this.maxNumbers,
            PI: Math.PI
        })
    }

    _updateParticleCountKernel
    _updateParticleStatusesKernel

    /**
     * Both minLife and maxLife are in seconds. So need to convert to frames beforeUse
     * @param activeParticleCount
     * @param initMaxVelocity
     * @param minLifeFrames
     * @param maxLifeFrames
     * @param currentFrameId
     */
    updateParticleStatuses(activeParticleCount, initMaxVelocity, minLifeFrames, maxLifeFrames, currentFrameId) {
        if (this._updateParticleCountKernel == null) {
            this._updateParticleCountKernel = huahuoEngine.ti.kernel(() => {
                currentActiveParticleNumber[0] = 0
                let i = 0
                while (i < maxNumbers) {  // while is not parallel. So no need to lock
                    if (particles[i].status == 1) {
                        currentActiveParticleNumber[0] += 1
                    }
                    i += 1
                }
            })
        }

        if (this._updateParticleStatusesKernel == null) {

            function initParticle(i, v, life, curFrameId) {
                let theta = ti.random() * 2 * PI
                let phi = ti.random() * PI
                let radius = ti.sqrt(ti.random())

                particles[i].velocity = 2.0 * [
                    ti.sin(phi) * ti.cos(theta) * radius * v[0],
                    ti.sin(phi) * ti.sin(theta) * radius * v[1],
                    ti.cos(phi) * radius * v[2]
                ] // random vector in the incircle of [-1, -1, -1] - [1, 1, 1]

                particles[i].position = [0.0, 0.0, 0.0]

                particles[i].status = 1

                particles[i].bornFrameId = curFrameId
                particles[i].lastUpdatedFrameId = curFrameId // Need to be updated anyways
                particles[i].life = i32(life)
            }

            function particleIsAlive(i, curFrameId) {
                return particles[i].status == 1 && particles[i].bornFrameId <= curFrameId && particles[i].bornFrameId + particles[i].life >= curFrameId
            }

            huahuoEngine.ti.addToKernelScope({initParticle, particleIsAlive})

            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._updateParticleStatusesKernel = huahuoEngine.ti.kernel(
                {initMaxVelocity: vType},
                (activeParticleCount, initMaxVelocity, minLifeFrames, maxLifeFrames, curFrameId) => {
                    let currentInactiveParticleNumber = maxNumbers - currentActiveParticleNumber[0]
                    let tobeActivatedParticleNumber = activeParticleCount - currentActiveParticleNumber[0]

                    if (tobeActivatedParticleNumber > 0) {
                        let possibility = tobeActivatedParticleNumber / currentInactiveParticleNumber

                        for (let i of range(maxNumbers)) {
                            if (particles[i].status == 0) {
                                let randomNumber = ti.random()
                                if (randomNumber <= possibility) {
                                    initParticle(i, initMaxVelocity, minLifeFrames + ti.random() * (maxLifeFrames - minLifeFrames), i32(curFrameId))
                                }
                            }
                        }
                    }
                })
        }

        this._updateParticleCountKernel()
        this._currentActiveParticleNumber.toArray().then((val) => {
            console.log("Expected active particle count:" + activeParticleCount + ", before active particles:" + val)
        })

        this._updateParticleStatusesKernel(activeParticleCount, initMaxVelocity, minLifeFrames, maxLifeFrames, currentFrameId)
        this._updateParticleCountKernel()
        this._currentActiveParticleNumber.toArray().then((val) => {
            console.log("Expected active particle count:" + activeParticleCount + ", actually active particles:" + val)
        })
    }

    _updateParticlesKernel

    updateParticles(mass, dt, curFrameId) {
        if (this._updateParticlesKernel == null) {
            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._updateParticlesKernel = huahuoEngine.ti.kernel(
                {gravity: vType},
                (gravity, dt, curFrameId) => {
                    for (let i of range(maxNumbers)) {
                        // Ensure the particle is born in this frame
                        if (particleIsAlive(i, curFrameId)) {
                            let timeElapseDirection = 1
                            if (curFrameId < particles[i].lastUpdatedFrameId) {
                                timeElapseDirection = -1
                            }

                            let signedDt = timeElapseDirection * dt

                            let curUpdatingFrameId = particles[i].lastUpdatedFrameId

                            while (curUpdatingFrameId != curFrameId) {
                                // -----   Use implicit Euler to update position.   -----
                                let nextVelocity = particles[i].velocity + signedDt * gravity

                                let possibleVelocity = (nextVelocity + particles[i].velocity) / 2.0
                                particles[i].position = particles[i].position + possibleVelocity * signedDt

                                // Update the velocity
                                particles[i].velocity = possibleVelocity

                                curUpdatingFrameId += timeElapseDirection
                            }

                            particles[i].lastUpdatedFrameId = i32(curFrameId)
                        } else {
                            particles[i].status = 0
                        }
                    }
                })
        }

        this._updateParticlesKernel(Vector3ToArray(this.gravity), dt, curFrameId)
    }

    _renderImageKernel

    renderImage(curFrameId) {
        if (this._renderImageKernel == null) {
            let cType = huahuoEngine.ti.types.vector(ti.f32, 4)

            this._renderImageKernel = huahuoEngine.ti.kernel(
                {particleColor: cType},
                (particleSize, particleColor, curFrameId) => {

                    let viewPortXMin = -outputImageWidth / 2;
                    let viewPortYMin = -outputImageHeight / 2;
                    for (let i of range(maxNumbers)) {
                        if (particleIsAlive(i, curFrameId)) {
                            // projection. For simplicity, ignore z coordinate first.
                            let projectedPosition = particles[i].position.xy

                            // TODO: https://www.geeksforgeeks.org/window-to-viewport-transformation-in-computer-graphics-with-implementation/
                            let centerWindowPosition = i32(projectedPosition - [viewPortXMin, viewPortYMin])

                            let particleSizeSquare = f32(particleSize * particleSize / 4.0)
                            for (let pixelIndex of ndrange(particleSize, particleSize)) {
                                let windowPosition = i32(centerWindowPosition + pixelIndex - [particleSize / 2, particleSize / 2])
                                if ((f32(windowPosition) - f32(centerWindowPosition)).norm_sqr() <= particleSizeSquare) {
                                    if (windowPosition[0] >= 0 && windowPosition[0] <= outputImageWidth && windowPosition[1] >= 0 && windowPosition[1] <= outputImageHeight)
                                        outputImage[windowPosition] = particleColor
                                }
                            }
                        }
                    }
                })
        }

        this._renderImageKernel(this.particleSize, ColorToArray(this.particleColor), curFrameId)

        // // For debug purpose, get the position array back
        // this._particlePositions.toArray().then(function (val) {
        //     console.log(val)
        // })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrameId = this.baseShape.getLayer().GetCurrentFrame()

        if (force || this.lastUpdatedFrameId != currentFrameId) {

            let minLife = Math.min(this.lifeSpanRange.x, this.lifeSpanRange.y)
            let maxLife = Math.max(this.lifeSpanRange.x, this.lifeSpanRange.y)

            // Set particle statuses.
            this.updateParticleStatuses(this.activeParticleCount, Vector3ToArray(this.initMaxVelocity), minLife * GlobalConfig.fps, maxLife * GlobalConfig.fps, currentFrameId)

            let dt = 1 / GlobalConfig.fps
            this.updateParticles(this.particleMass, dt, currentFrameId)

            // // Check particles status again.
            // this.updateParticleStatuses(this.activeParticleCount, Vector3ToArray(this.initMaxVelocity), this.maxLife * GlobalConfig.fps, currentFrameId)

            this.lastUpdatedFrameId = currentFrameId
        }
    }
}

export {Particles}