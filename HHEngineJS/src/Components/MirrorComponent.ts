import {AbstractComponent, propertyValue} from "./AbstractComponent";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyCategory} from "./PropertySheetBuilder";

class MirrorComponent extends AbstractComponent{
    @propertyValue(PropertyCategory.shapeArray)
    targetShapeArray: Array<BaseShapeJS>
}

export {MirrorComponent}