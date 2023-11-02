import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {huahuoEngine, PropertyCategory, renderEngine2D} from "../index";
import {FloatPropertyConfig} from "hhcommoncomponents";

@Component()
class Camera2D extends AbstractComponent {
    @PropertyValue(PropertyCategory.interpolateFloat, 0.0, {min: 0.0, max: 1.0, step: 0.01} as FloatPropertyConfig)
    margin

    _rectangle

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (huahuoEngine.getActivePlayer().isPlaying){ // Only Focus when playing.
            renderEngine2D.getCameraBox().hide()
            renderEngine2D.setViewPosition(this.baseShape.position)
        }
        else{ // Show margin box.
            renderEngine2D.getCameraBox().setMargin(this.margin)
            renderEngine2D.getCameraBox().show()
        }
    }
}

export {Camera2D}