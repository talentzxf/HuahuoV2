import {MATERIAL_TYPE} from "./World";
import {BaseShape} from "./Shapes";

class Hose extends BaseShape {
    center = [0.2, 0.8]
    radius = 0.02

    totalParticles = 3000
    hoseEjectTime = 10.0
    materialId = MATERIAL_TYPE.LIQUID

    startTime

    startRecycleTime = null
    lastRecycleTime = null
    recycleTime = 10.0

    velocity = [5.0, 1.0]

    constructor() {
        super();

        this.startTime = Date.now()
    }

    addParametersToKernel() {
        super.addParametersToKernel();
        ti.addToKernelScope({
            center: this.center,
            radius: this.radius,
            hoseVelocity: this.velocity
        })
    }

    nextVelocity() {
        return (total, idx)=>{
            return hoseVelocity
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
        if(currentHosePercentage < 1.0){
            this.setActivePercentage(Math.min(1.0, currentHosePercentage))
        }

        if(currentHosePercentage >= 1.0){
            if(this.lastRecycleTime == null){
                this.lastRecycleTime = Date.now()
                this.startRecycleTime = Date.now()
            }else{
                let recycleTimeMili = this.recycleTime * 1000.0
                let lastElapsedTimeSinceStart = this.lastRecycleTime - this.startRecycleTime
                let currentElapsedTimeSinceStart = Date.now() - this.startRecycleTime

                let lastIdx = Math.floor((lastElapsedTimeSinceStart % recycleTimeMili) / recycleTimeMili * this.totalParticles)
                let currentIdx = Math.floor(currentElapsedTimeSinceStart % recycleTimeMili/ recycleTimeMili * this.totalParticles)

                console.log("Recycling:" + lastIdx+":"+currentIdx)

                this.resetKernel(lastIdx, currentIdx, this.materialId, 0.0, 0.0)

                this.lastRecycleTime = Date.now()
            }
        }
    }
}

export {Hose}