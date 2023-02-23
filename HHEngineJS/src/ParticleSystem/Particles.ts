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
    _particleStatuses // 0 - inactive, 1 - active
    _particlePositions
    _particleVelocity

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
        this._particleVelocity = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particlePositions = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particleStatuses = huahuoEngine.ti.field(ti.f32, [this.maxNumbers])

        huahuoEngine.ti.addToKernelScope({
            particleVelocity: this._particleVelocity,
            particlePositions: this._particlePositions,
            particleStatuses: this._particleStatuses,
            maxNumbers: this.maxNumbers,
            PI: Math.PI
        })
    }

    _updateParticleStatusesKernel

    updateParticleStatuses(activeParticleCount) {
        if (this._updateParticleStatusesKernel == null) {
            this._updateParticleStatusesKernel = huahuoEngine.ti.kernel((activeParticleCount) => {
                for (let i of range(maxNumbers)) {
                    if (i < activeParticleCount) {
                        particleStatuses[i] = 1
                    } else {
                        particleStatuses[i] = 0
                    }
                }
            })
        }

        this._updateParticleStatusesKernel(activeParticleCount)
    }

    _initParticlesKernel

    initParticles(velocity) {
        if (this._initParticlesKernel == null) {
            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._initParticlesKernel = huahuoEngine.ti.kernel(
                {v: vType},
                (v) => {
                    for (let i of range(maxNumbers)) {
                        if (particleStatuses[i] == 1) {

                            let theta = ti.random() * 2 * PI
                            let phi = ti.random() * PI
                            let radius = ti.sqrt(ti.random())

                            particleVelocity[i] = 2.0 * [
                                ti.sin(phi) * ti.cos(theta) * radius * v[0],
                                ti.sin(phi) * ti.sin(theta) * radius * v[1],
                                ti.cos(phi) * radius * v[2]
                            ] // random vector in the incircle of [-1, -1, -1] - [1, 1, 1]
                        }

                        particlePositions[i] = [0.0, 0.0, 0.0]
                    }
                })
        }
        this._initParticlesKernel(velocity);
    }

    _updateParticlesKernel

    updateParticles(mass, dt) {
        if (this._updateParticlesKernel == null) {
            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._updateParticlesKernel = huahuoEngine.ti.kernel(
                {gravity: vType},
                (gravity, dt) => {
                for (let i of range(maxNumbers)) {
                    if (particleStatuses[i] == 1) {
                        // Use implicit Euler to update position.
                        let nextVelocity = particleVelocity[i] + dt * gravity

                        let possibleVelocity = (nextVelocity + particleVelocity[i])/2.0
                        particlePositions[i] = particlePositions[i] + possibleVelocity * dt

                        // Update the velocity
                        particleVelocity[i] = possibleVelocity
                    }
                }
            })
        }

        this._updateParticlesKernel(Vector3ToArray(this.gravity), dt)
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
                        if (particleStatuses[i] == 1) {
                            // projection. For simplicity, ignore z coordinate first.
                            let projectedPosition = [particlePositions[i][0], particlePositions[i][1]]

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

        this._renderImageKernel(this.particleSize, ColorToArray(this.particleColor))

        // // For debug purpose, get the position array back
        // this._particlePositions.toArray().then(function (val) {
        //     console.log(val)
        // })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrameId = this.baseShape.getLayer().GetCurrentFrame()

        if (force || this.lastUpdatedFrameId != currentFrameId) {
            // Set particle statuses.
            this.updateParticleStatuses(this.activeParticleCount)

            if (this.lastUpdatedFrameId == -1) {
                this.initParticles(Vector3ToArray(this.initMaxVelocity))
            }

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