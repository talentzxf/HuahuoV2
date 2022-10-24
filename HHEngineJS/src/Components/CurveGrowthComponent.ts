import {AbstractComponent, interpolateValue} from "./AbstractComponent";

class CurveGrowthComponent extends AbstractComponent{
    @interpolateValue(1.0)
    growth: number;

    lastGrowthNumber: number = -1.0
    clonedPaperShape

    get paperShape(){
        return this.baseShape.paperShape
    }

    getSegments(){
        if(!this.clonedPaperShape){
            return this.paperShape.segments
        }
        return this.clonedPaperShape.segments
    }

    setGrowth(growth: number){
        this.rawObj.SetValue("growth", growth)
    }

    override afterUpdate() {
        super.afterUpdate();

        if(this.growth < 1.0){
            this.clonedPaperShape = this.paperShape.clone()
            this.clonedPaperShape.selected = false
            this.clonedPaperShape.visible = false
            this.clonedPaperShape.data = null

            let path2 = this.paperShape.splitAt(this.paperShape.length * this.growth)
            if(path2){
                path2.visible = false
                path2.selected = false
                path2.remove()
            }

            this.lastGrowthNumber = this.growth
        }
    }
}

export {CurveGrowthComponent}