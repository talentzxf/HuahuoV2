import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {huahuoEngine} from "../EngineAPI";

@Component({compatibleShapes:["BaseShapeJS"]})
class Motor extends AbstractComponent{

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 1.0, y: 0.0})
    velocity

    // TODO: Add Time unit.
    afterUpdate(force: boolean = false) {
        if(huahuoEngine.getActivePlayer().isPlaying){
            super.afterUpdate(force);

            let action = this.baseShape.getAction()
            let currentPosition = this.baseShape.position
            let nextPosition = currentPosition.add(this.velocity)
            action.setPosition(nextPosition.x, nextPosition.y)
        }
    }
}

export {Motor}