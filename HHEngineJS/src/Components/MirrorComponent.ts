import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

class MirrorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray
}

export {MirrorComponent}