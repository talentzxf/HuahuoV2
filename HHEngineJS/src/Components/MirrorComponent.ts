import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

class MirrorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    mirroredShapes: Array<BaseShapeJS>

    constructor(rawObj?) {
        super(rawObj)

        // Register target shape change
        this
    }

}

export {MirrorComponent}