import {AbstractComponent, interpolateValue} from "./AbstractComponent";
import {CurveShapeJS} from "../Shapes/CurveShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";

let componentName = "CurveGrowthComponent"
class CurveGrowthComponent extends AbstractComponent{
    static createComponent(rawObj){
        new CurveGrowthComponent(rawObj)
    }

    @interpolateValue(1.0)
    growth: number;

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

    override afterUpdate() {
        super.afterUpdate();

        let growth = this["getGrowth"]()
        if( growth < 1.0){
            this.clonedPaperShape = this.paperShape.clone()
            this.clonedPaperShape.selected = false
            this.clonedPaperShape.visible = false
            this.clonedPaperShape.data = null

            let path2 = this.paperShape.splitAt(this.paperShape.length * growth)
            if(path2){
                path2.visible = false
                path2.selected = false
                path2.remove()
            }
        }
    }
}

clzObjectFactory.RegisterClass(componentName, CurveGrowthComponent.createComponent)

export {CurveGrowthComponent}