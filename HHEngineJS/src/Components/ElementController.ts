import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {GraphAction} from "../EventGraph/GraphActions";

@Component({compatibleShapes: ["ElementShapeJS"], maxCount: 1})
class ElementController extends AbstractComponent {
    @PropertyValue(PropertyCategory.interpolateFloat, 1.0)
    playSpeed: number;

    @GraphAction(true)
    setFrameId(){

    }
}
export {ElementController}

