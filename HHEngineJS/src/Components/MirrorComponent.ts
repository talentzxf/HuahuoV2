import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {mirrorPoint} from "hhcommoncomponents";
import * as paper from "paper"

class MirrorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    targetShapeMirroredShapeMap: Map<BaseShapeJS, BaseShapeJS> = new Map<BaseShapeJS, BaseShapeJS>()

    paperShapeGroup: paper.Group
    p1: paper.Point
    p2: paper.Point
    constructor(rawObj?) {
        super(rawObj)

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape
    }

    setStartPoint(p1){
        this.p1 = p1
    }

    setEndPoint(p2){
        this.p2 = p2
    }

    afterUpdate() {
        super.afterUpdate();

        if(this.targetShapeArray){
            // Check if all target shapes are mirrored
            for(let targetShape of this.targetShapeArray){
                if(!this.targetShapeMirroredShapeMap.has(targetShape)){
                    let duplicatedShape = targetShape.duplicate()

                    // Move the position to the target position.
                    duplicatedShape.position = mirrorPoint(duplicatedShape.position,
                        this.p1, this.p2)

                    duplicatedShape.setSelectedMeta(this.baseShape)

                    this.targetShapeMirroredShapeMap.set(targetShape, duplicatedShape)
                }
            }
        }
    }

}

export {MirrorComponent}