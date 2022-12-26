import {MATERIAL_TYPE} from "./World";

let quality = 1

class BaseShape {
    check_boundary_kernel
    totalParticles = 500 * quality ** 2;
    E = 8e2; // Young's modulus a
    nu = 0.01; // Poisson's ratio

    mu_0 = this.E / (2 * (1 + this.nu));
    lambda_0 = (this.E * this.nu) / ((1 + this.nu) * (1 - 2 * this.nu)); // Lame parameters

    startIdx = -1
    endIdx = -1
    materialId = MATERIAL_TYPE.SOFTBODY

    resetKernel

    reloading = false

    activeKernel

    activePercentageKernel

    setActivePercentage(activePercentage) {
        if (this.activePercentageKernel == null) {
            this.activePercentageKernel = ti.kernel((startIdx, activeEndIdx, totalParticleCount) => {
                for (let i of range(totalParticleCount)) {
                    let particleIndex = i32(startIdx + i)
                    if (i < activeEndIdx)
                        active[particleIndex] = 1
                    else
                        active[particleIndex] = 0
                }
            })
        }

        let totalParticles = this.endIdx - this.startIdx
        this.activePercentageKernel(this.startIdx, this.startIdx + totalParticles * activePercentage, this.totalParticles)
    }

    setActive(isActive) {
        if (this.activeKernel == null) {
            this.activeKernel = ti.kernel((startIdx, endIdx, isActive) => {
                let totalParticleCount = endIdx - startIdx
                for (let i of range(totalParticleCount)) {
                    let particleIndex = i32(startIdx + i)
                    active[particleIndex] = i32(isActive)
                }
            })
        }

        this.activeKernel(this.startIdx, this.endIdx, isActive)
    }

    constructor() {
    }

    addToShapeManager(shapeManager) {
        let [startIdx, endIdx] = shapeManager.addShape(this)

        this.startIdx = startIdx
        this.endIdx = endIdx
    }

    nextPosition() {
        return (total, idx) => {
            return [ti.random() * 0.2 + 0.3, ti.random() * 0.2 + 0.05]
        }
    }

    nextVelocity() {
        return (total, idx) => {
            return [0.0, 0.0]
        }
    }

    nextMaterial() {
        return (total, idx, materialId) => {
            return materialId
        }
    }

    colorFunc() {
        return (total, idx, materialId) => {
            let this_color = f32([0, 0, 0, 0])
            if (materialId == 0) {
                this_color = [0, 0.5, 0.5, 1.0];
            } else if (materialId == 1) {
                this_color = [0.93, 0.33, 0.23, 1.0];
            } else if (materialId == 2) {
                this_color = [1, 1, 1, 1.0];
            }

            return this_color
        }
    }

    async addParametersToKernel() {
        let _this = this
        ti.addToKernelScope({
            nextPositionFunc: _this.nextPosition(),
            nextVelocityFunc: _this.nextVelocity(),
            nextMaterialFunc: _this.nextMaterial(),
            colorFunc: _this.colorFunc(),
        })
    }

    apply_constraint() {

    }

    invoke_check_boundary_kernel(startIdx, endIdx) {
        if (this.check_boundary_kernel == null) {
            this.check_boundary_kernel = ti.kernel((startIdx, endIdx) => {
                let minY = 100.0

                let totalParticles = endIdx - startIdx
                for (let i of ti.range(totalParticles)) {
                    let particleIdx = i32(startIdx + i)

                    if (x[particleIdx][1] < minY) {
                        minY = x[particleIdx][1]
                    }
                }

                return minY
            })
        }

        return this.check_boundary_kernel(startIdx, endIdx)
    }

    check_boundary() {

    }

    async reset(offsetX = 0.0, offsetY = 0.0) {
        if (this.reloading)
            return false

        this.reloading = true
        let _this = this

        await this.addParametersToKernel()

        if (!this.resetKernel) {
            this.resetKernel = ti.kernel(
                (startParticleIndex,
                 endParticleIndex,
                 materialId, offsetX, offsetY) => {
                    let totalParticleCount = i32(endParticleIndex - startParticleIndex)
                    for (let i of range(totalParticleCount)) {
                        let particleIndex = i32(startParticleIndex + i)
                        let point = nextPositionFunc(totalParticleCount, i) + [offsetX, offsetY];
                        x[particleIndex] = point

                        material[particleIndex] = i32(nextMaterialFunc(totalParticleCount, i, materialId))
                        v[particleIndex] = nextVelocityFunc(totalParticleCount, i)
                        F[particleIndex] = [
                            [1, 0],
                            [0, 1]
                        ]

                        Jp[i] = 100.0
                        C[i] = [
                            [0, 0],
                            [0, 0]
                        ]

                        particle_color[particleIndex] = colorFunc(totalParticleCount, i, materialId)
                    }

                    return true
                })
        }
        console.log("Resetting kernel:" + _this.startIdx + "," + _this.endIdx)

        let resetKernelPromise = this.resetKernel(_this.startIdx, _this.endIdx, _this.materialId, offsetX, offsetY)
        resetKernelPromise.then((val) => {
            if (val)
                _this.reloading = false
            else {
                console.log("Why why why?")
            }
        })

        this.setActive(true)

        return true
    }

    update() {

    }
}

class CircleShape extends BaseShape {
    center = [0.5, 0.5]
    radius = 0.05

    handleRadius = this.radius / 10.0
    handleCenter = [0.5, 0.5]

    addVelocityKernel

    addVelocity(velocity) {
        console.log("Adding velocity:" + velocity)

        if (this.addVelocityKernel == null) {
            this.addVelocityKernel = ti.kernel((startIdx, endIdx, velocityX, velocityY) => {
                let totalParticles = endIdx - startIdx
                for (let i of ti.range(totalParticles)) {
                    let particleIdx = i32(startIdx + i)
                    x[particleIdx] += [velocityX, velocityY]

                    // v[particleIdx] += [velocityX, velocityY]
                    //
                    // let base = i32(x[particleIdx] * inv_dx - 0.5);
                    // for (let i of ti.static(ti.range(3))) {
                    //     for (let j of ti.static(ti.range(3))) {
                    //         grid_v[base + [i, j]] += [velocityX, velocityY]
                    //     }
                    // }
                }
            })
        }

        // let handleStartIdx = this.startIdx + this.totalParticles * 15.0/16.0
        // let handleEndIdx = this.endIdx
        this.addVelocityKernel(this.startIdx, this.endIdx, velocity[0], velocity[1])
    }

    check_boundary() {
        this.invoke_check_boundary_kernel(this.startIdx, this.endIdx)
    }

    addParametersToKernel() {
        super.addParametersToKernel();
        ti.addToKernelScope({
            center: this.center,
            radius: this.radius,
            handleRadius: this.handleRadius,
            handleCenter: this.handleCenter
        })
    }

    update() {

    }

    nextPosition() {
        return (total, idx) => {

            let retPosition = [-1.0, -1.0]

            if (idx * 16.0 < 15.0 * total) { // 8/9 particles are inside the circle.
                let theta = ti.random() * 2.0 * Math.PI
                let r = radius * ti.sqrt(ti.random())
                retPosition = [center[0] + r * ti.sin(theta), center[1] + r * ti.cos(theta)]
            } else { // Other particles are concentracted in the handle part.
                let theta = ti.random() * 2.0 * Math.PI
                let r = handleRadius * ti.sqrt(ti.random())
                retPosition = [handleCenter[0] + r * ti.sin(theta), handleCenter[1] + r * ti.cos(theta)]
            }

            return retPosition
        }
    }
}

class BoardShape extends BaseShape {
    center = [0.1, 0.5]
    length = 0.1
    height = 0.02

    handleVelocity = [0.0, 10.0]

    velocityConstraintFunc

    constructor() {
        super();
    }

    addParametersToKernel() {
        super.addParametersToKernel();
        ti.addToKernelScope({
            center: this.center,
            boardLength: this.length,
            boardHeight: this.height,
            handleVelocity: this.handleVelocity
        })
    }

    check_boundary() {
        let _this = this
        this.invoke_check_boundary_kernel(this.startIdx, this.endIdx).then((val) => {
            if (this.reloading)
                return

            if (val > 0.8) {
                let currentCenterX = _this.center[0]

                let nextCenterX = (Math.random() - 0.5) * 0.5 + 0.4

                console.log("Proposed center x:" + nextCenterX)
                console.log("Current center:" + currentCenterX + "Offset:" + (nextCenterX - currentCenterX))
                _this.reset(nextCenterX - currentCenterX, 0.0)
            }
        })
    }

    apply_constraint() {
        if (this.velocityConstraintFunc == null) {
            // Fix handle's velocity
            this.velocityConstraintFunc = ti.kernel((startIdx, endIdx) => {
                let totalParticles = endIdx - startIdx
                for (let i of ti.range(totalParticles)) {
                    let particleIdx = i32(startIdx + i)
                    v[particleIdx] = handleVelocity

                    let base = i32(x[particleIdx] * inv_dx - 0.5);
                    for (let i of ti.static(ti.range(3))) {
                        for (let j of ti.static(ti.range(3))) {
                            grid_v[base + [i, j]] = handleVelocity
                        }
                    }
                }
            })
        }

        let leftHandleStartIdx = this.startIdx + this.totalParticles * 8.0 / 9.0
        let rightHandleEndIdx = this.endIdx
        this.velocityConstraintFunc(leftHandleStartIdx, rightHandleEndIdx)
    }

    nextPosition() {
        return (total, idx) => {
            let resultPosition = [-1.0, -1.0]
            if (idx * 9.0 < 8.0 * total) {
                // Leave 8/9 in the inner and keep 1/9 as handles.
                resultPosition = [
                    ti.random() * boardLength + center[0] - boardLength / 2.0,
                    ti.random() * boardHeight + center[1] - boardHeight / 2.0
                ]
            } else if (idx * 18.0 < 17.0 * total) { // Left handle
                resultPosition = [center[0] - boardLength / 2.0, ti.random() * boardHeight + center[1] - boardHeight / 2.0]
            } else { // Right handle
                resultPosition = [center[0] + boardLength / 2.0, ti.random() * boardHeight + center[1] - boardHeight / 2.0]
            }

            return resultPosition
        }
    }
}

export {CircleShape, BaseShape, BoardShape}