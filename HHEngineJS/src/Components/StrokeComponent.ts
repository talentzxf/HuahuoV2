import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {FloatPropertyConfig} from "hhcommoncomponents";

@Component()
class StrokeComponent extends AbstractComponent{

    @PropertyValue(PropertyCategory.interpolateFloat, 1, {min: 0.0} as FloatPropertyConfig)
    strokeWidth

    @PropertyValue(PropertyCategory.interpolateColor, {r:0, g:0, b:0, a:1})
    strokeColor

    override afterUpdate() {
        super.afterUpdate();

        this.baseShape.paperShape.strokeColor = this.strokeColor
        this.baseShape.paperShape.strokeWidth = this.strokeWidth
    }
}

export {StrokeComponent}