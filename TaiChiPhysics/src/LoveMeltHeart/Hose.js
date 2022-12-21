import {MATERIAL_TYPE} from "./World";
import {BaseShape} from "./Shapes";

class Hose extends BaseShape {
    center = [0.5, 0.5]
    radius = 0.05

    materialId = MATERIAL_TYPE.LIQUID

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

export {Hose}