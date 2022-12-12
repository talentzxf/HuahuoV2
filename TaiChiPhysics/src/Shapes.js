import {quality} from "./Constants";
import {MATERIAL_TYPE} from "./ShapeManager";

class BaseShape {
    totalParticles = 1000 * quality ** 2;
    E = 8e2; // Young's modulus a
    nu = 0.01; // Poisson's ratio

    mu_0 = this.E / (2 * (1 + this.nu));
    lambda_0 = (this.E * this.nu) / ((1 + this.nu) * (1 - 2 * this.nu)); // Lame parameters

    startIdx = -1
    endIdx = -1
    materialId = MATERIAL_TYPE.SOFTBODY

    resetKernel

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

    addParametersToKernel() {
        let _this = this
        ti.addToKernelScope({
            nextPositionFunc: _this.nextPosition()
        })
    }

    apply_constraint(){

    }

    reset() {
        let _this = this

        this.addParametersToKernel()

        if (!this.resetKernel) {
            this.resetKernel = ti.kernel(
                (startParticleIndex,
                 endParticleIndex,
                 materialId) => {
                    let totalParticleCount = i32(endParticleIndex - startParticleIndex)
                    for (let i of range(totalParticleCount)) {
                        let particleIndex = i32(startParticleIndex + i)
                        let point = nextPositionFunc(totalParticleCount, i);
                        x[particleIndex] = point

                        material[particleIndex] = i32(materialId)
                        v[particleIndex] = [0, 0]
                        F[particleIndex] = [
                            [1, 0],
                            [0, 1]
                        ]

                        Jp[i] = 1.0
                        C[i] = [
                            [0, 0],
                            [0, 0]
                        ]
                    }
                })
        }
        console.log("Resetting kernel:" + _this.startIdx + "," + _this.endIdx)
        this.resetKernel(_this.startIdx, _this.endIdx, _this.materialId)
    }
}

class CircleShape extends BaseShape {
    center = [0.5, 0.5]
    radius = 0.05

    addParametersToKernel() {
        super.addParametersToKernel();
        ti.addToKernelScope({
            center: this.center,
            radius: this.radius
        })
    }

    nextPosition() {
        return (total, idx) => {
            let theta = ti.random() * 2.0 * Math.PI
            let r = radius * ti.sqrt(ti.random())

            return [center[0] + r * ti.sin(theta), center[1] + r * ti.cos(theta)]
        }
    }
}

class BoardShape extends BaseShape {
    center = [0.1, 0.5]
    length = 0.2
    height = 0.02

    velocityConstraintFunc

    addParametersToKernel() {
        super.addParametersToKernel();
        ti.addToKernelScope({
            center: this.center,
            boardLength: this.length,
            boardHeight: this.height
        })
    }

    apply_constraint(){
        if(this.velocityConstraintFunc == null){
            // Fix handle's velocity
            this.velocityConstraintFunc = ti.kernel((startIdx, endIdx)=>{
                let totalParticles = endIdx - startIdx
                for(let i of ti.range(totalParticles)){
                    let particleIdx = i32(startIdx + i)
                    v[particleIdx] = [0.0, 0.0]

                    let base = i32(x[particleIdx] * inv_dx - 0.5);
                    for (let i of ti.static(ti.range(3))) {
                        for (let j of ti.static(ti.range(3))) {
                            grid_v[base + [i, j]] = [0.0, 0.0]
                        }
                    }
                }
            })
        }

        let leftHandleStartIdx = this.startIdx + this.totalParticles * 8.0/9.0
        let rightHandleEndIdx = this.endIdx
        this.velocityConstraintFunc(leftHandleStartIdx, rightHandleEndIdx)
    }

    nextPosition() {
        return (total, idx) => {
            let resultPosition = [-1.0, -1.0]
            if (idx * 9.0 < 8.0 * total){
                // Leave 8/9 in the inner and keep 1/9 as handles.
                resultPosition = [
                    ti.random() * boardLength + center[0] - boardLength / 2.0,
                    ti.random() * boardHeight + center[1] - boardHeight / 2.0
                ]
            }
            else if (idx * 18.0 < 17.0 * total) { // Left handle
                resultPosition = [center[0] - boardLength / 2.0, ti.random() * boardHeight + center[1] - boardHeight / 2.0]
            } else {
                resultPosition = [center[0] + boardLength / 2.0, ti.random() * boardHeight + center[1] - boardHeight / 2.0]
            }

            return resultPosition
        }
    }
}

export {CircleShape, BaseShape, BoardShape}