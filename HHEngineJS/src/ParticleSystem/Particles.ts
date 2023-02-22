// @ts-nocheck

import {huahuoEngine} from "../EngineAPI";
import {AbstractComponent, Component, PropertyValue} from "../Components/AbstractComponent";
import {PropertyCategory} from "../Components/PropertySheetBuilder";
import {GlobalConfig} from "../GlobalConfig";

const MAX_PARTICLE_COUNT = 1000

function Vector3ToArray(vec:Vector3){
    return [vec.x, vec.y, vec.z]
}

// ParticleSystem is not compatible with any shape. It should be used in ParticleSystemRenderer only as a subcomponent.
@Component({compatibleShapes: []})
class Particles extends AbstractComponent {
    _particleStatuses // 0 - inactive, 1 - active
    _particlePositions
    _particleVelocity

    @PropertyValue(PropertyCategory.interpolateVector3, {x:100.0, y:100.0, z: 100.0})
    initMaxVelocity

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
            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._initParticlesKernel = huahuoEngine.ti.kernel(
                {v: vType},
                (v) => {
                for (let i of range(maxNumbers)) {
                    if (particleStatuses[i] == 1) {
                        particleVelocity[i] = 2.0 * [
                            (ti.random() - 0.5) * v[0],
                            (ti.random() - 0.5) * v[1],
                            (ti.random() - 0.5) * v[2]
                        ] // random vector in [-1, -1, -1] - [1, 1, 1]
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
                this.initParticles(Vector3ToArray(this.initMaxVelocity))
            }

            // TODO: Split into fixed physical frames.
            let dt = (currentFrameId - this.lastUpdatedFrameId) / GlobalConfig.fps
            this.updateParticles(this.particleMass, dt)

            this.lastUpdatedFrameId = currentFrameId
        }
    }
}

export {Particles}