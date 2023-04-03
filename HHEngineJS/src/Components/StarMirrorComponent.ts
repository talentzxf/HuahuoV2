import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import * as paper from "paper";

class StarMirrorComponent extends AbstractComponent{

    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    @PropertyValue(PropertyCategory.interpolateFloat, 120.0)
    mirrorInterval

    paperShapeGroup: paper.Group

    constructor(rawObj?) {
        super(rawObj);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape
    }
}

export {StarMirrorComponent}