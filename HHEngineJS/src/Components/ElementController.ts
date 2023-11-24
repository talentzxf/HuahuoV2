import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {ActionParam, GraphAction} from "../EventGraph/GraphActions";
import {HHToast, PropertyType} from "hhcommoncomponents";
import {ElementShapeJS} from "../Shapes/ElementShapeJS";
import {StringProperty} from "hhcommoncomponents";

@Component({compatibleShapes: ["ElementShapeJS"], maxCount: 1})
class ElementController extends AbstractComponent {
    @PropertyValue(PropertyCategory.interpolateFloat, 1.0)
    playSpeed: number;

    @PropertyValue(PropertyCategory.stringValue, "playWithGlobal", {options:["playWithGlobal", "playLocally"]} as StringProperty)
    playMode: string

    @GraphAction(true)
    setFrameId(@ActionParam(PropertyType.NUMBER) playFrameId) {
        if (playFrameId <= 0) {
            HHToast.error("Invalid argument:" + playFrameId)
            return
        }
        let elementShape = this.baseShape as ElementShapeJS
        elementShape.setPlayerFrameId(playFrameId - 1) // The UI frameId starts from 1, but internally starts from 0.
    }

    reset() {
        super.reset()
    }
}

export {ElementController}

