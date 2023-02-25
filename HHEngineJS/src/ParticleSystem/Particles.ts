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
            startUpFrameId: huahuoEngine.ti.i32,
            lastUpdatedFrameId: huahuoEngine.ti.i32
        })

        this._particles = huahuoEngine.ti.field(particleType, [this.maxNumbers])

        this._currentActiveParticleNumber = huahuoEngine.ti.field(ti.i32, [1])

        huahuoEngine.ti.addToKernelScope({
            particles: this._particles,
            currentActiveParticleNumber: this._currentActiveParticleNumber,
            maxNumbers: this.maxNumbers,
            PI: Math.PI
        })
    }

    _updateActiveParticleCountKernel
    _updateParticleStatusesKernel

    updateParticleStatuses(activeParticleCount, initMaxVelocity, curFrameId) {

        if (this._updateActiveParticleCountKernel == null) {
            this._updateActiveParticleCountKernel = huahuoEngine.ti.kernel(() => {
                currentActiveParticleNumber[0] = 0
                for (let i of ti.static(range(maxNumbers))) { // Check the active particle count
                    if (particles[i].status == 1) {
                        ti.atomicAdd(currentActiveParticleNumber[0], 1)
                    }
                }
            })
        }

        if (this._updateParticleStatusesKernel == null) {

            function initParticle(i, v) {
                let theta: f32 = ti.random() * 2 * PI
                let phi: f32 = ti.random() * PI
                let radius: f32 = ti.sqrt(ti.random())

                particles[i].velocity = 2.0 * [
                    ti.sin(phi) * ti.cos(theta) * radius * v[0],
                    ti.sin(phi) * ti.sin(theta) * radius * v[1],
                    ti.cos(phi) * radius * v[2]
                ] // random vector in the incircle of [-1, -1, -1] - [1, 1, 1]

                particles[i].position = [0.0, 0.0, 0.0]

                particles[i].status = 1
            }

            huahuoEngine.ti.addToKernelScope({initParticle})

            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._updateParticleStatusesKernel = huahuoEngine.ti.kernel(
                {initMaxVelocity: vType},
                (activeParticleCount, initMaxVelocity) => {
                    let currentInactiveParticleNumber: i32 = maxNumbers - currentActiveParticleNumber[0]
                    let tobeActivatedParticleNumber: i32 = activeParticleCount - currentActiveParticleNumber[0]

                    if (tobeActivatedParticleNumber > 0) {
                        let possibility: f32 = f32(tobeActivatedParticleNumber) / f32(currentInactiveParticleNumber)

                        for (let i: i32 of range(maxNumbers)) {
                            if (particles[i].status == 0) {
                                let randomNumber: f32 = ti.random()
                                if (randomNumber <= possibility) {
                                    initParticle(i, f32(initMaxVelocity))
                                }
                            }
                        }
                    }
                })
        }

        this._updateActiveParticleCountKernel()
        this._updateParticleStatusesKernel(activeParticleCount, initMaxVelocity)
    }

    _updateParticlesKernel

    updateParticles(mass, dt) {
        if (this._updateParticlesKernel == null) {
            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._updateParticlesKernel = huahuoEngine.ti.kernel(
                {gravity: vType},
                (gravity, dt) => {
                    for (let i of range(maxNumbers)) {
                        if (particles[i].status == 1) {
                            // -----   Use implicit Euler to update position.   -----
                            let nextVelocity = particles[i].velocity + dt * gravity

                            let possibleVelocity = (nextVelocity + particles[i].velocity) / 2.0
                            particles[i].position = particles[i].position + possibleVelocity * dt

                            // Update the velocity
                            particles[i].velocity = possibleVelocity
                        }
                    }
                })
        }

        return this._updateParticlesKernel(Vector3ToArray(this.gravity), dt)
    }

    _renderImageKernel

    renderImage() {
        if (this._renderImageKernel == null) {
            let cType = huahuoEngine.ti.types.vector(ti.f32, 4)

            this._renderImageKernel = huahuoEngine.ti.kernel(
                {particleColor: cType},
                (particleSize, particleColor) => {

                    let viewPortXMin = -outputImageWidth / 2;
                    let viewPortYMin = -outputImageHeight / 2;
                    for (let i of range(maxNumbers)) {
                        if (particles[i].status == 1) {
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
                                    else {
                                        // This particle is out of range. Mark it as inactive.
                                        particles[i].status = 0
                                    }
                                }
                            }
                        }
                    }
                })
        }

        this._renderImageKernel(this.particleSize, ColorToArray(this.particleColor))
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);
        let currentFrameId = this.baseShape.getLayer().GetCurrentFrame()

        if (force || this.lastUpdatedFrameId != currentFrameId) {
            // Set particle statuses.
            this.updateParticleStatuses(this.activeParticleCount, Vector3ToArray(this.initMaxVelocity), currentFrameId)

            let timeElapseDirection = currentFrameId > this.lastUpdatedFrameId ? 1 : -1;

            let dt = timeElapseDirection / GlobalConfig.fps
            for (let curUpdatingFrameId = this.lastUpdatedFrameId; curUpdatingFrameId != currentFrameId; curUpdatingFrameId += timeElapseDirection) {
                this.updateParticles(this.particleMass, dt)
            }

            this.lastUpdatedFrameId = currentFrameId
        }
    }
}

export {Particles}