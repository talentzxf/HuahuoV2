import {AbstractComponent, interpolateValue} from "./AbstractComponent";
import {CurveShapeJS} from "../Shapes/CurveShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";

let componentName = "CurveGrowthComponent"
class CurveGrowthComponent extends AbstractComponent{
    static createComponent(rawObj){
        return new CurveGrowthComponent(rawObj)
    }

    constructor(rawObj?) {
        super(rawObj);

        this.rawObj.SetTypeName(componentName)
    }

    @interpolateValue(1.0)
    growth: number;

    clonedPaperShape

    get paperShape(){
        return this.baseShape.paperShape
    }

    setSegmentProperty(idx, property, value){
        if(this.clonedPaperShape){
            let updatedLength = this.paperShape.length / this.clonedPaperShape.length
            this.growth = updatedLength
            this.clonedPaperShape.segments[idx].point = this.baseShape.paperShape.segments[idx].point
            this.clonedPaperShape.segments[idx].handleIn = this.baseShape.paperShape.segments[idx].handleIn
            this.clonedPaperShape.segments[idx].handleOut = this.baseShape.paperShape.segments[idx].handleOut
        }
    }

    getSegments(){
        if(!this.clonedPaperShape){
            return this.paperShape.segments
        }
        return this.clonedPaperShape.segments
    }

    // TODO, avoid duplication. This logic is the same as what we have in the BaseShapeJS.insertSegment.
    insertSegment(localPos){
        if(this.clonedPaperShape){
            let nearestPoint = this.clonedPaperShape.getNearestPoint(localPos)
            let offset = this.clonedPaperShape.getOffsetOf(nearestPoint)

            while (!this.clonedPaperShape.divideAt(offset)) {
                offset += 0.01 // Hit the corner points, offset a little and divide again.
            }
        }
    }

    override afterUpdate() {
        super.afterUpdate();

        let growth = this.growth
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