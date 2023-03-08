// @ts-nocheck

import {huahuoEngine} from "../EngineAPI";
import {AbstractComponent, Component, PropertyValue} from "../Components/AbstractComponent";
import {PropertyCategory} from "../Components/PropertySheetBuilder";
import {GlobalConfig} from "../GlobalConfig";
import {ParticleShapeLoader} from "./ParticleShapeLoader";

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
    particleShapeLoader: ParticleShapeLoader = new ParticleShapeLoader()

    _particles
    _currentActiveParticleNumber;

    @PropertyValue(PropertyCategory.customField)
    particleShape

    @PropertyValue(PropertyCategory.interpolateFloat, 10.0)
    particleSize

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 300.0, y: 500.0})
    velocityMagnitudeRange

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 0.0, y: 360.0})
    velocityThetaRange

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 0.0, y: 360.0})
    velocityPhiRange

    /**
     * Follow means particle's dir will be it's velocity's direction     *
     */
    @PropertyValue(PropertyCategory.stringValue, "velocity")
    particleDirection

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 1.0, y: 3.0}) // Unit is seconds.
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

        // Handle the particle shape part, as it's a custom field. We have to handle everything by our selves.
        if (rawObj == null) { // If this is a new object, register the field and init the value.
            this.rawObj.RegisterBinaryResource("particleShape")
            this.rawObj.SetBinaryResourceName("particleShape", "") // Empty string is a placeholder.
        }

        this.valueChangeHandler.registerValueChangeHandler("*")(this.propertyChanged.bind(this))
        this.valueChangeHandler.registerValueChangeHandler("particleShape")(this.particleShapeChanged.bind(this))
    }

    particleShapeChanged(imgName) {
        let fieldName = "particleShape"
        this.rawObj.SetBinaryResourceName(fieldName, imgName)
    }

    _invalidateAllParticlesKernel

    propertyChanged() {
        if (this._invalidateAllParticlesKernel == null) {
            this._invalidateAllParticlesKernel = huahuoEngine.ti.kernel(() => {
                for (let i of range(maxNumbers)) {
                    particles[i].status = 0
                }
            })
        }

        this._invalidateAllParticlesKernel()
        this.afterUpdate(true)
    }

    _updateParticleCountKernel
    _updateParticleStatusesKernel

    /**
     * Both minLife and maxLife are in seconds. So need to convert to frames beforeUse
     * @param activeParticleCount
     * @param velocityRange
     * @param velocityThetaRange
     * @param velocityPhiRange
     * @param minLifeFrames
     * @param maxLifeFrames
     * @param currentFrameId
     */
    updateParticleStatuses(activeParticleCount, velocityRange, velocityThetaRange, velocityPhiRange, minLifeFrames, maxLifeFrames, currentFrameId) {
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

            function initParticle(i, vMagRange, vThetaRange, vPhiRange, life, curFrameId) {
                let theta = ti.random() * (vThetaRange[1] - vThetaRange[0]) + vThetaRange[0]
                let phi = ti.random() * (vPhiRange[1] - vPhiRange[0]) + vPhiRange[0] // Rotate phi 180 degrees, so if phi=0, the particles face the viewer.
                let radius = ti.sqrt(ti.random()) * (vMagRange[1] - vMagRange[0]) + vMagRange[0]

                theta = theta * Math.PI / 180  // Convert degree to radian.
                phi = phi * Math.PI / 180 // Convert degree to radian.

                particles[i].velocity = 2.0 * [
                    ti.cos(phi) * ti.cos(theta) * radius,
                    ti.cos(phi) * ti.sin(theta) * radius,
                    ti.sin(phi) * radius
                ]

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
                {velocityRange: vType, velocityThetaRange: vType, velocityPhiRange: vType},
                (activeParticleCount, velocityRange, velocityThetaRange, velocityPhiRange, minLifeFrames, maxLifeFrames, curFrameId) => {
                    let currentInactiveParticleNumber = maxNumbers - currentActiveParticleNumber[0]
                    let tobeActivatedParticleNumber = activeParticleCount - currentActiveParticleNumber[0]

                    if (tobeActivatedParticleNumber > 0) {
                        let possibility = tobeActivatedParticleNumber / currentInactiveParticleNumber

                        for (let i of range(maxNumbers)) {
                            if (particles[i].status == 0) {
                                let randomNumber = ti.random()
                                if (randomNumber <= possibility) {
                                    initParticle(i, velocityRange, velocityThetaRange, velocityPhiRange, minLifeFrames + ti.random() * (maxLifeFrames - minLifeFrames), i32(curFrameId))
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

        this._updateParticleStatusesKernel(activeParticleCount, velocityRange, velocityThetaRange, velocityPhiRange, minLifeFrames, maxLifeFrames, currentFrameId)
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

            let num_triangles = MAX_PARTICLE_COUNT * 2 // Each particle has two triangles.
            let vertexType = huahuoEngine.ti.types.struct({
                pos: huahuoEngine.ti.types.vector(ti.f32, 3)
            });
            let particle_vertices = huahuoEngine.ti.field(vertexType, [num_triangles * 2]) // Each particle has two triangles (4 vertices)
            let particle_indices = huahuoEngine.ti.field(huahuoEngine.ti.i32, [num_triangles * 3]);

            huahuoEngine.ti.addToKernelScope({
                particle_count: MAX_PARTICLE_COUNT,
                num_triangles,
                particle_vertices,
                particle_indices
            })
            let set_indices = huahuoEngine.ti.kernel(() => {
                for (let i of ti.range(particle_count)) {
                    // First triangle
                    particle_indices[i * 6] = i * 4
                    particle_indices[i * 6 + 1] = i * 4 + 1
                    particle_indices[i * 6 + 2] = i * 4 + 2

                    // Second triangle
                    particle_indices[i * 6 + 3] = i * 4 + 2
                    particle_indices[i * 6 + 4] = i * 4 + 3
                    particle_indices[i * 6 + 5] = i * 4
                }
            })

            set_indices()

            this._renderImageKernel = huahuoEngine.ti.kernel(
                {particleColor: cType},
                (particleSize, particleColor, curFrameId, velocityDir, staticDir) => {

                    let center = [0.0, 0.0, 0]
                    let eye = [0.0, 0.0, 10.0]
                    let fov = 45
                    let view = ti.lookAt(eye, center, [0.0, 1.0, 0.0])
                    let proj = ti.perspective(fov, aspectRatio, 0.1, 1000)
                    let mvp = proj.matmul(view)

                    ti.clearColor(renderTarget, [0.1, 0.2, 0.3, 1])
                    ti.useDepth(depth)

                    // set up vertices of all the particles.
                    for(let i of range(maxNumbers) ){
                        if(particleIsAlive(i, curFrameId)){ //
                            let particlePosition = particles[i].position
                            let particleVelocityXY = particles[i].velocity.normalized().xy

                            particle_vertices[4*i].pos = [particlePosition[0] - 0.5 * particleVelocityXY[0] , particlePosition[1] + 0.5 * particleVelocityXY[1], particlePosition[2]]
                            particle_vertices[4*i + 1].pos = [particlePosition[0] + 0.5 * particleVelocityXY[0] , particlePosition[1] + 0.5 * particleVelocityXY[1], particlePosition[2]]
                            particle_vertices[4*i + 2].pos = [particlePosition[0] + 0.5 * particleVelocityXY[0] , particlePosition[1] - 0.5 * particleVelocityXY[1], particlePosition[2]]
                            particle_vertices[4*i + 3].pos = [particlePosition[0] - 0.5 * particleVelocityXY[0] , particlePosition[1] - 0.5 * particleVelocityXY[1], particlePosition[2]]

                        }else{
                            particle_vertices[4*i].pos = [0.0, 0.0, 0.0]
                            particle_vertices[4*i + 1].pos = [0.0, 0.0, 0.0]
                            particle_vertices[4*i + 2].pos = [0.0, 0.0, 0.0]
                            particle_vertices[4*i + 3].pos = [0.0, 0.0, 0.0]
                        }
                    }

                    // Vertex shader
                    for(let v of ti.inputVertices(particle_vertices, particle_indices)){
                        let pos = mvp.matmul(v.pos.concat([1.0]))
                        ti.outputPosition(pos)
                        ti.outputVertex(v)
                    }

                    // Fragment shader
                    for(let f of ti.inputFragments()){
                        ti.outputColor(renderTarget, [1.0, 0.0, 1.0, 1.0])
                    }

                    // let viewPortXMin = -outputImageWidth / 2;
                    // let viewPortYMin = -outputImageHeight / 2;
                    // for (let i of range(maxNumbers)) {
                    //     if (particleIsAlive(i, curFrameId)) {
                    //         // projection. For simplicity, ignore z coordinate first.
                    //         let projectedPosition = particles[i].position.xy
                    //
                    //         // TODO: https://www.geeksforgeeks.org/window-to-viewport-transformation-in-computer-graphics-with-implementation/
                    //         let centerWindowPosition = i32(projectedPosition - [viewPortXMin, viewPortYMin])
                    //
                    //         if (particleShapeSize[0] <= 0 || particleShapeSize[0] <= 0) { // Draw circle
                    //             let particleSizeSquare = f32(particleSize * particleSize / 4.0)
                    //             for (let pixelIndex of ndrange(particleSize, particleSize)) {
                    //                 let windowPosition = i32(centerWindowPosition + pixelIndex - [particleSize / 2, particleSize / 2])
                    //                 if ((f32(windowPosition) - f32(centerWindowPosition)).norm_sqr() <= particleSizeSquare) {
                    //                     if (windowPosition[0] >= 0 && windowPosition[0] <= outputImageWidth && windowPosition[1] >= 0 && windowPosition[1] <= outputImageHeight) {
                    //                         outputImage[windowPosition] = particleColor
                    //                     }
                    //                 }
                    //             }
                    //         } else { // Draw shape from the particleShape image.
                    //
                    //
                    //             for (let pixelIndex of ndrange(particleSize, particleSize)) {
                    //                 let windowPosition = i32(centerWindowPosition + pixelIndex - [particleSize / 2, particleSize / 2])
                    //
                    //                 // TODO: Only consider x-y axis for now.
                    //                 let particleVelocity = particles[i].velocity.xy
                    //
                    //                 // let angle = ti.atan2(particleVelocity[1], particleVelocity[0])
                    //                 let angle = 30.0 / 180.0 * Math.PI
                    //                 let pixelAfterRotateX = pixelIndex[0] * ti.cos(angle) - pixelIndex[1] * ti.sin(angle)
                    //                 let pixelAfterRotateY = pixelIndex[0] * ti.sin(angle) + pixelIndex[1] * ti.cos(angle)
                    //
                    //                 let imgPositionX = i32(particleShapeSize[0] * pixelAfterRotateX / particleSize)
                    //                 let imgPositionY = i32(particleShapeSize[1] * pixelAfterRotateY / particleSize)
                    //
                    //                 let particleShapeColor = particleShapeData[particleShapeSize[1] - imgPositionY, imgPositionX]
                    //                 if (particleShapeColor[3] > 0.0) {
                    //                     outputImage[windowPosition] = particleColor
                    //
                    //                     // TODO: How to do alpha blending??
                    //                     // let alpha = particleShapeColor[3]
                    //                     //
                    //                     // // alpha * new + (1-alpha)*old
                    //                     // let currentRGB = outputImage[windowPosition].rgb
                    //                     // let newRGB = alpha * particleColor.rgb + (1.0 - alpha) * currentRGB
                    //                     //
                    //                     // outputImage[windowPosition] = [newRGB[0], newRGB[1], newRGB[2], alpha]
                    //                 }
                    //             }
                    //         }
                    //     }
                    // }
                })
        }

        let velocityDir = 0
        let staticDir = 0
        if (this.particleDirection.startsWith("velocity")) { // This means the dir is the same as the dir of the velocity.
            let splittedDir = this.particleDirection.split("velocity")
            velocityDir = Number(splittedDir[1])
        } else {
            staticDir = Number(this.particleDirection)
        }

        this._renderImageKernel(this.particleSize, ColorToArray(this.particleColor), curFrameId, velocityDir, staticDir)

        // // For debug purpose, get the position array back
        // this._particlePositions.toArray().then(function (val) {
        //     console.log(val)
        // })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrameId = this.baseShape.getLayer().GetCurrentFrame()

        if (force || this.lastUpdatedFrameId != currentFrameId) {

            this.particleShapeLoader.setParticleShape(this.rawObj.GetBinaryResource("particleShape"))

            let minLife = Math.min(this.lifeSpanRange.x, this.lifeSpanRange.y)
            let maxLife = Math.max(this.lifeSpanRange.x, this.lifeSpanRange.y)

            // Set particle statuses.
            this.updateParticleStatuses(this.activeParticleCount, Vector3ToArray(this.velocityMagnitudeRange), Vector3ToArray(this.velocityThetaRange),
                Vector3ToArray(this.velocityPhiRange), minLife * GlobalConfig.fps, maxLife * GlobalConfig.fps, currentFrameId)

            let dt = 1 / GlobalConfig.fps
            this.updateParticles(this.particleMass, dt, currentFrameId)

            // // Check particles status again.
            // this.updateParticleStatuses(this.activeParticleCount, Vector3ToArray(this.initMaxVelocity), this.maxLife * GlobalConfig.fps, currentFrameId)

            this.lastUpdatedFrameId = currentFrameId
        }
    }
}

export {Particles}