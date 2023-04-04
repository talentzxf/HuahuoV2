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

    @PropertyValue(PropertyCategory.interpolateFloat, 0.5)
    particleSize

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 3.0, y: 5.0})
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

    @PropertyValue(PropertyCategory.interpolateVector3, {x: 0.0, y: -9.8, z: 0.0})
    gravity

    maxNumbers = MAX_PARTICLE_COUNT // Preload MAX_PARTICLE_COUNT particles.

    lastUpdatedFrameId = -1;


    // This buffer is only used for IDE. During runtime, no need to have this one.
    frameDataBuffers

    constructor(rawObj?, isMirage = false) {
        super(rawObj, isMirage)

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
            maxNumbers: this.maxNumbers,
            PI: Math.PI
        })

        // Handle the particle shape part, as it's a custom field. We have to handle everything by our selves.
        if (rawObj == null || !rawObj.IsFieldRegistered("particleShape")) { // If this is a new object, register the field and init the value.
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
            this._invalidateAllParticlesKernel = huahuoEngine.ti.classKernel(this, () => {
                for (let i of range(maxNumbers)) {
                    this._particles[i].status = 0
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
            this._updateParticleCountKernel = huahuoEngine.ti.classKernel(this, () => {
                this._currentActiveParticleNumber[0] = 0
                let i = 0
                while (i < maxNumbers) {  // while is not parallel. So no need to lock
                    if (this._particles[i].status == 1) {
                        this._currentActiveParticleNumber[0] += 1
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

                this._particles[i].velocity = 2.0 * [
                    ti.cos(phi) * ti.cos(theta) * radius,
                    ti.cos(phi) * ti.sin(theta) * radius,
                    ti.sin(phi) * radius
                ]

                this._particles[i].position = [0.0, 0.0, 0.0]

                this._particles[i].status = 1

                this._particles[i].bornFrameId = curFrameId
                this._particles[i].lastUpdatedFrameId = curFrameId // Need to be updated anyways
                this._particles[i].life = i32(life)
            }

            function particleIsAlive(i, curFrameId) {
                return this._particles[i].status == 1 && this._particles[i].bornFrameId <= curFrameId && this._particles[i].bornFrameId + this._particles[i].life >= curFrameId
            }

            huahuoEngine.ti.addToKernelScope({initParticle, particleIsAlive})

            let vType = huahuoEngine.ti.types.vector(ti.f32, 3)

            this._updateParticleStatusesKernel = huahuoEngine.ti.classKernel(this,
                {velocityRange: vType, velocityThetaRange: vType, velocityPhiRange: vType},
                (activeParticleCount, velocityRange, velocityThetaRange, velocityPhiRange, minLifeFrames, maxLifeFrames, curFrameId) => {
                    let currentInactiveParticleNumber = maxNumbers - this._currentActiveParticleNumber[0]
                    let tobeActivatedParticleNumber = activeParticleCount - this._currentActiveParticleNumber[0]

                    if (tobeActivatedParticleNumber > 0) {
                        let possibility = tobeActivatedParticleNumber / currentInactiveParticleNumber

                        for (let i of range(maxNumbers)) {
                            if (this._particles[i].status == 0) {
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

            this._updateParticlesKernel = huahuoEngine.ti.classKernel(this,
                {gravity: vType},
                (gravity, dt, curFrameId) => {
                    for (let i of range(maxNumbers)) {
                        // Ensure the particle is born in this frame
                        if (particleIsAlive(i, curFrameId)) {
                            let timeElapseDirection = 1
                            if (curFrameId < this._particles[i].lastUpdatedFrameId) {
                                timeElapseDirection = -1
                            }

                            let signedDt = timeElapseDirection * dt

                            let curUpdatingFrameId = this._particles[i].lastUpdatedFrameId

                            while (curUpdatingFrameId != curFrameId) {
                                // -----   Use implicit Euler to update position.   -----
                                let nextVelocity = this._particles[i].velocity + signedDt * gravity

                                let possibleVelocity = (nextVelocity + this._particles[i].velocity) / 2.0
                                this._particles[i].position = this._particles[i].position + possibleVelocity * signedDt

                                // Update the velocity
                                this._particles[i].velocity = possibleVelocity

                                curUpdatingFrameId += timeElapseDirection
                            }

                            this._particles[i].lastUpdatedFrameId = i32(curFrameId)
                        } else {
                            this._particles[i].status = 0
                        }
                    }
                })
        }

        this._updateParticlesKernel(Vector3ToArray(this.gravity), dt, curFrameId)
    }

    _renderImageKernel

    _particle_vertices
    _particle_indices

    setupIndicesAndVertices() {
        let num_triangles = MAX_PARTICLE_COUNT * 2 // Each particle has two triangles.
        let vertexType = huahuoEngine.ti.types.struct({
            pos: huahuoEngine.ti.types.vector(ti.f32, 3),
            center: huahuoEngine.ti.types.vector(ti.f32, 3), // The center of the particle
            texture_pos: ti.types.vector(ti.f32, 2),
        });
        this._particle_vertices = huahuoEngine.ti.field(vertexType, [num_triangles * 2]) // Each particle has two triangles (4 vertices)
        this._particle_indices = huahuoEngine.ti.field(huahuoEngine.ti.i32, [num_triangles * 3]);

        let set_indices = huahuoEngine.ti.classKernel(this, (particle_count) => {
            for (let i of ti.range(particle_count)) {
                // First triangle
                this._particle_indices[i * 6] = i * 4
                this._particle_indices[i * 6 + 1] = i * 4 + 1
                this._particle_indices[i * 6 + 2] = i * 4 + 2

                // Second triangle
                this._particle_indices[i * 6 + 3] = i * 4 + 2
                this._particle_indices[i * 6 + 4] = i * 4 + 3
                this._particle_indices[i * 6 + 5] = i * 4
            }
        })

        set_indices(MAX_PARTICLE_COUNT)
    }

    depth
    renderTarget

    renderImage(curFrameId, aspectRatio) {
        if (this._renderImageKernel == null) {
            this.setupIndicesAndVertices()

            let cType = huahuoEngine.ti.types.vector(ti.f32, 4)
            let shapeSizeType = huahuoEngine.ti.template()
            let textureType = huahuoEngine.ti.template()

            this._renderImageKernel = huahuoEngine.ti.classKernel(
                this,
                {particleColor: cType, particleShapeSize: shapeSizeType, particleShapeTexture: textureType},
                (particleSize, particleColor, curFrameId, particleShapeSize, particleShapeTexture, velocityDir, staticDir, aspectRatio) => {

                    let invalidPosition = [-10000.0, -10000.0, -10000.0]

                    let particleSizeSquare = f32(particleSize * particleSize / 4.0)

                    let center = [0.0, 0.0, 0]
                    let eye = [0.0, 0.0, 10.0]
                    let fov = 45
                    let view = ti.lookAt(eye, center, [0.0, 1.0, 0.0])
                    let proj = ti.perspective(fov, aspectRatio, 0.1, 1000)
                    let mvp = proj.matmul(view)

                    // ti.clearColor(renderTarget, [0.0, 0.0, 0.0, 0.0])
                    ti.useDepth(this.depth)

                    let halfParticleSize = particleSize * 0.5
                    // set up vertices of all the particles.
                    for (let i of range(maxNumbers)) {
                        if (particleIsAlive(i, curFrameId)) { //
                            let particlePosition = this._particles[i].position
                            // let particleVelocityXY = particles[i].velocity.normalized().xy
                            let particleVelocityXY = [1.0, 1.0]

                            this._particle_vertices[4 * i].pos = [particlePosition[0] - halfParticleSize * particleVelocityXY[0], particlePosition[1] + halfParticleSize * particleVelocityXY[1], particlePosition[2]]
                            this._particle_vertices[4 * i + 1].pos = [particlePosition[0] + halfParticleSize * particleVelocityXY[0], particlePosition[1] + halfParticleSize * particleVelocityXY[1], particlePosition[2]]
                            this._particle_vertices[4 * i + 2].pos = [particlePosition[0] + halfParticleSize * particleVelocityXY[0], particlePosition[1] - halfParticleSize * particleVelocityXY[1], particlePosition[2]]
                            this._particle_vertices[4 * i + 3].pos = [particlePosition[0] - halfParticleSize * particleVelocityXY[0], particlePosition[1] - halfParticleSize * particleVelocityXY[1], particlePosition[2]]

                            this._particle_vertices[4 * i].center = particlePosition.xyz
                            this._particle_vertices[4 * i + 1].center = particlePosition.xyz
                            this._particle_vertices[4 * i + 2].center = particlePosition.xyz
                            this._particle_vertices[4 * i + 3].center = particlePosition.xyz
                        } else {
                            this._particle_vertices[4 * i].pos = invalidPosition
                            this._particle_vertices[4 * i + 1].pos = invalidPosition
                            this._particle_vertices[4 * i + 2].pos = invalidPosition
                            this._particle_vertices[4 * i + 3].pos = invalidPosition

                            this._particle_vertices[4 * i].center = invalidPosition
                            this._particle_vertices[4 * i + 1].center = invalidPosition
                            this._particle_vertices[4 * i + 2].center = invalidPosition
                            this._particle_vertices[4 * i + 3].center = invalidPosition
                        }

                        this._particle_vertices[4 * i].texture_pos = [0.0, 0.0]
                        this._particle_vertices[4 * i + 1].texture_pos = [1.0, 0.0]
                        this._particle_vertices[4 * i + 2].texture_pos = [1.0, 1.0]
                        this._particle_vertices[4 * i + 3].texture_pos = [0.0, 1.0]
                    }

                    // Vertex shader
                    for (let v of ti.inputVertices(this._particle_vertices, this._particle_indices)) {
                        let pos = mvp.matmul(v.pos.concat([1.0]))
                        ti.outputPosition(pos)
                        ti.outputVertex(v)
                    }

                    // Fragment shader
                    for (let f of ti.inputFragments()) {
                        // Draw a circle on the triangle
                        let fragmentPos = f.pos
                        let centerPos = f.center

                        if (particleShapeSize[0] > 0 && particleShapeSize[1] > 0) { // Draw texture
                            let textureColor = ti.textureSample(particleShapeTexture, f.texture_pos)
                            if (textureColor[3] > 0)
                                ti.outputColor(this.renderTarget, particleColor)
                            else
                                ti.discard()
                        } else { // Draw circle.
                            if ((fragmentPos - centerPos).norm_sqr() <= particleSizeSquare) {
                                ti.outputColor(this.renderTarget, particleColor)
                            } else {
                                ti.discard()
                            }
                        }
                    }
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

        this._renderImageKernel(this.particleSize, ColorToArray(this.particleColor), curFrameId,
            this.particleShapeLoader._particleShapeSize, this.particleShapeLoader._particleShapeTexture,
            velocityDir, staticDir, aspectRatio)

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