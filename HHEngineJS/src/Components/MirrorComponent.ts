import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

class MirrorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    targetShapeMirroredShapeMap: Map<BaseShapeJS, BaseShapeJS> = new Map<BaseShapeJS, BaseShapeJS>()

    paperShapeGroup: paper.Group
    constructor(rawObj?) {
        super(rawObj)

        let paper = this.getPaperJs()
        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this
    }

    afterUpdate() {
        super.afterUpdate();

        // Check if all target shapes are mirrored
        for(let targetShape of this.targetShapeArray){
            if(!this.targetShapeMirroredShapeMap.has(targetShape)){
                let duplicatedShape = targetShape.duplicate()

                // Move the position to the target position.
                duplicatedShape.position =
            }
        }
    }

}

export {MirrorComponent}