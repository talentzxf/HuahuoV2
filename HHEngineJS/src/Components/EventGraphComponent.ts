import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseShapeEvents} from "../EventGraph/BaseShapeEvents";

@Component()
class EventGraphComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.stringValue, null, null, true)
    eventGraphJSON

    @PropertyValue(PropertyCategory.customField)
    eventGraph

    // TODO: Need persist of following arrays.
    actions:Map<BaseShapeJS, BaseShapeActions> = new Map

    eventEmitters: Map<BaseShapeJS, BaseShapeEvents> = new Map

    getAction(baseShape: BaseShapeJS){
        if(!this.actions.has(baseShape)){
            this.actions.set(baseShape, new BaseShapeActions(baseShape))
        }
        return this.actions.get(baseShape)
    }

    getEvent(baseShape: BaseShapeJS){
        if(!this.eventEmitters.has(baseShape)){
            this.eventEmitters.set(baseShape, new BaseShapeEvents(baseShape))
        }
        return this.eventEmitters.get(baseShape)
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        this.actions.forEach(action=>{action.execute()})
    }
}

export {EventGraphComponent}