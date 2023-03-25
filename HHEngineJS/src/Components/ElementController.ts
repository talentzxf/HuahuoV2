import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {FloatPropertyConfig} from "hhcommoncomponents";

@Component({compatibleShapes: ["ElementShapeJS"], maxCount: 1})
class ElementController extends AbstractComponent {
    @PropertyValue(PropertyCategory.interpolateFloat, 1.0)
    playSpeed: number;
}
export {ElementController}