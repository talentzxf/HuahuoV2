// @ts-nocheck

import {huahuoEngine} from "../EngineAPI";
import {AbstractComponent, Component, PropertyValue} from "../Components/AbstractComponent";
import {PropertyCategory} from "../Components/PropertySheetBuilder";
import {GlobalConfig} from "../GlobalConfig";

const MAX_PARTICLE_COUNT = 1000

// ParticleSystem is not compatible with any shape. It should be used in ParticleSystemRenderer only as a subcomponent.
@Component({compatibleShapes: []})
class Particles extends AbstractComponent {
    _particleStatuses // 0 - inactive, 1 - active
    _particlePositions
    _particleVelocity

    @PropertyValue(PropertyCategory.interpolateFloat, 100, {max: MAX_PARTICLE_COUNT})
    activeParticleCount

    @PropertyValue(PropertyCategory.interpolateFloat, 100)
    particleMass

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
            maxNumbers: this.maxNumbers
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
            this._initParticlesKernel = huahuoEngine.ti.kernel((initMaxVelocityX, initMaxVelocityY, initMaxVelocityZ) => {
                for (let i of range(maxNumbers)) {
                    if (particleStatuses[i] == 1) {
                        particleVelocity[i] = 2.0 * [
                            (ti.random() - 0.5) * initMaxVelocityX,
                            (ti.random() - 0.5) * initMaxVelocityY,
                            (ti.random() - 0.5) * initMaxVelocityZ
                        ] // random vector in [-1, -1, -1] - [1, 1, 1]
                    }

                    particlePositions[i] = [0.0, 0.0, 0.0]
                }
            })
        }
        this._initParticlesKernel(velocity[0], velocity[1], velocity[2])
    }

    _updateParticlesKernel

    updateParticles(mass, dt) {
        if (this._updateParticlesKernel == null) {
            this._updateParticlesKernel = huahuoEngine.ti.kernel((mass, dt) => {
                for (let i of range(maxNumbers)) {
                    if (particleStatuses[i] == 1) {
                        particlePositions[i] = particlePositions[i] + particleVelocity[i] * dt
                    }
                }
            })
        }

        this._updateParticlesKernel(mass, dt)
    }

    _renderImageKernel

    renderImage() {
        if (this._renderImageKernel == null) {
            this._renderImageKernel = huahuoEngine.ti.kernel(() => {

                let viewPortXMin = -outputImageWidth / 2;
                let viewPortYMin = -outputImageHeight / 2;
                for (let i of range(maxNumbers)) {
                    if (particleStatuses[i] == 1) {
                        // projection. For simplicity, ignore z coordinate first.
                        let projectedPosition = [particlePositions[i][0], particlePositions[i][1]]

                        // TODO: https://www.geeksforgeeks.org/window-to-viewport-transformation-in-computer-graphics-with-implementation/
                        let windowPosition = i32(projectedPosition - [viewPortXMin, viewPortYMin])
                        outputImage[windowPosition] = [1.0, 0.0, 0.0, 1.0]
                    }
                }
            })
        }

        this._renderImageKernel()

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
                this.initParticles([100.0, 100.0, 100.0])
            }

            // TODO: Split into fixed physical frames.
            let dt = (currentFrameId - this.lastUpdatedFrameId) / GlobalConfig.fps
            this.updateParticles(this.particleMass, dt)

            this.lastUpdatedFrameId = currentFrameId
        }
    }
}

export {Particles}