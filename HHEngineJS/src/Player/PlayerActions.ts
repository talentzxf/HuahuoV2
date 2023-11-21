import {AbstractGraphAction, ActionParam, GraphAction} from "../EventGraph/GraphActions";
import {HHToast, PropertyType} from "hhcommoncomponents"
import {huahuoEngine} from "../EngineAPI";
import {ElementShapeJS} from "../Shapes/ElementShapeJS";

declare var Module: any;

class PlayerActions extends AbstractGraphAction {
    graphComponent

    constructor(graphComponent) {
        super()
        this.graphComponent = graphComponent
    }

    @GraphAction()
    jumpToFrame(@ActionParam(PropertyType.NUMBER) frameId, @ActionParam(PropertyType.STRING) frameName) {
        let realFrameId = frameId - 1 // Again, in UI, frameId starts from 1. Internall, frameId starts from 0.

        if (isNaN(realFrameId) || realFrameId < 0) {
            realFrameId = this.graphComponent.baseShape.getLayer().GetFrameIdByName(frameName)

            if(realFrameId < 0){
                HHToast.warn("Invalid param")
            }
        }

        // If the shape is within an element, we need to control the elements' element controller.
        // If not, we need to control the actionPlayer.
        let baseShape = this.graphComponent.baseShape
        if (baseShape.getParent() != null) {
            if (this.graphComponent.baseShape.getParent().getRawObject() instanceof Module.ElementShape) {
                let elementShapeJS = this.graphComponent.baseShape.getParent() as ElementShapeJS
                elementShapeJS.elementController.setFrameId(realFrameId + 1) // As element controller also receive UI frameId, set UI frameId here.
            } else {
                HHToast.warning("Shape parent is not element???")
            }
        } else {
            huahuoEngine.getActivePlayer().setFrameId(realFrameId)
        }

    }
}

export {PlayerActions}