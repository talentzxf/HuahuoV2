import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

@Component({compatibleShapes:["BaseSolidShape"], maxCount: 1})
class RadialGradientComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.colorStopArray)
    gradientColorArray
}

export {RadialGradientComponent}