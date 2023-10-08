import {AbstractGraphAction, ActionParam, GraphAction} from "../EventGraph/GraphActions";
import {HHToast, PropertyType} from "hhcommoncomponents"

class PlayerActions extends AbstractGraphAction {
    baseShape
    constructor(baseShape) {
        super()
        this.baseShape = baseShape
    }

    @GraphAction()
    jumpToFrame(@ActionParam(PropertyType.NUMBER) frameId) {
        if (frameId <= 0) {
            HHToast.warn("Invalid param")
        }
        // If the shape is within an element, we need to control the elements' element controller.

        // If not, we need to control the actionPlayer.
    }
}

export {PlayerActions}