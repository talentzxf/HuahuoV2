import {AbstractComponent, Component} from "./AbstractComponent";
import {huahuoEngine, renderEngine2D} from "../index";

@Component()
class Camera2D extends AbstractComponent {
    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(huahuoEngine.getActivePlayer().isPlaying) // Only Focus when playing.
            renderEngine2D.setViewPosition(this.baseShape.position)
    }
}

export {Camera2D}