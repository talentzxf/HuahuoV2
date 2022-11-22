import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {getNailManager, Nail} from "../IK/NailManager";

@Component({compatibleShapes:["BaseSolidShape"], maxCount: 1})
class NailComponent extends AbstractComponent{
    nails: Array<Nail> = new Array<Nail>()

    // The coordinate of this hitPoint is in global world pos.
    addNail(nail: Nail){
        this.nails.push(nail)
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrame = this.baseShape.getLayer().GetCurrentFrame()
        getNailManager().updateAllNails(currentFrame)
    }
}

export {NailComponent}