import {MATERIAL_TYPE} from "./World";
import {BaseShape} from "./Shapes";

class Hose extends BaseShape {
    center = [0.5, 0.5]
    radius = 0.01

    totalParticles = 1000
    hoseEjectTime = 10.0
    materialId = MATERIAL_TYPE.LIQUID

    startTime

    startRecycleTime = null
    lastRecycleTime = null
    recycleTime = 10.0

    constructor() {
        super();

        this.startTime = Date.now()
    }

    addParametersToKernel() {
        super.addParametersToKernel();
        ti.addToKernelScope({
            center: this.center,
            radius: this.radius
        })
    }

    nextVelocity() {
        return (total, idx)=>{
            return [5.0, 1.0]
        }
    }

    nextPosition() {
        return (total, idx) => {
            let theta = ti.random() * 2.0 * Math.PI
            let r = radius * ti.sqrt(ti.random())
            return [center[0] + r * ti.sin(theta), center[1] + r * ti.cos(theta)]
        }
    }

    update() {
        super.update()
        let currentHosePercentage = (Date.now() - this.startTime) / 1000.0 / this.hoseEjectTime
        this.setActivePercentage(Math.min(1.0, currentHosePercentage))
        if(currentHosePercentage >= 1.0){
            if(this.lastRecycleTime == null){
                this.lastRecycleTime = Date.now()
                this.startRecycleTime = Date.now()
            }else{
                let recycleTimeMili = this.recycleTime * 1000.0
                let lastElapsedTimeSinceStart = this.lastRecycleTime - this.startRecycleTime
                let currentElapsedTimeSinceStart = Date.now() - this.startRecycleTime

                let lastIdx = (lastElapsedTimeSinceStart % recycleTimeMili) / recycleTimeMili * this.totalParticles
                let currentIdx = currentElapsedTimeSinceStart % recycleTimeMili/ recycleTimeMili * this.totalParticles

                console.log("Recycling:" + lastIdx+":"+currentIdx)
                this.resetKernel(lastIdx, currentIdx, this.materialId, 0.0, 0.0)

                this.lastRecycleTime = Date.now()
            }
        }
    }
}

export {Hose}