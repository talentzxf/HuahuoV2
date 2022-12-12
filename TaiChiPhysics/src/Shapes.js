import {quality} from "./Constants";
import {MATERIAL_TYPE} from "./ShapeManager";

class BaseShape{
    totalParticles = 9000 * quality ** 2;
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

    addToShapeManager(shapeManager){
        let [startIdx, endIdx] = shapeManager.addShape(this)

        this.startIdx = startIdx
        this.endIdx = endIdx
    }

    nextPosition(){
        return ()=>{
            return [ ti.random() * 0.2 + 0.3, ti.random() * 0.2 + 0.05]
        }
    }

    addParametersToKernel(){
        let _this = this
        ti.addToKernelScope({
            nextPositionFunc: _this.nextPosition()
        })
    }

    reset(){
        let _this = this

        this.addParametersToKernel()

        if(!this.resetKernel){
            this.resetKernel = ti.kernel(
                (startParticleIndex,
                       endParticleIndex,
                       materialId)=>{
                let totalParticleCount = i32(endParticleIndex - startParticleIndex)
                for(let i of range(totalParticleCount)){
                    let particleIndex = i32(startParticleIndex + i)
                    let point = nextPositionFunc();
                    x[particleIndex] = point

                    material[particleIndex] = i32(materialId)
                    v[particleIndex] = [0,0]
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

class CircleShape extends BaseShape{
    center = [0.5, 0.5]
    radius = 0.1

    addParametersToKernel() {
        super.addParametersToKernel();
        ti.addToKernelScope({
            center: this.center,
            radius: this.radius
        })
    }

    nextPosition(){
        return ()=>{
            let theta = ti.random() * 2.0 * Math.PI
            let r = radius * ti.sqrt( ti.random() )

            return [center[0] + r * ti.sin(theta), center[1] + r * ti.cos(theta)]
        }
    }
}

export {CircleShape, BaseShape}