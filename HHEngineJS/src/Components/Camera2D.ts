import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {huahuoEngine, PropertyCategory, renderEngine2D} from "../index";
import {FloatPropertyConfig} from "hhcommoncomponents";

@Component()
class Camera2D extends AbstractComponent {
    @PropertyValue(PropertyCategory.interpolateFloat, 0.0, {min: 0.0, max: 1.0, step: 0.01} as FloatPropertyConfig)
    margin

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (huahuoEngine.getActivePlayer().isPlaying){ // Only Focus when playing.
            renderEngine2D.hideCameraBox()
            renderEngine2D.setViewPosition(this.baseShape.position)
        }
        else{ // Show margin box.
            renderEngine2D.setCameraBoxMargin(this.margin)
        }
    }
}

export {Camera2D}