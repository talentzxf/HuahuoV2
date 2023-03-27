import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

@Component()
class EventGraphComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.stringValue, null, null, true)
    eventGraphJSON

    @PropertyValue(PropertyCategory.customField)
    eventGraph

    actions:Map<BaseShapeJS, BaseShapeActions> = new Map

    getAction(baseShape: BaseShapeJS){
        if(!this.actions.has(baseShape)){
            this.actions.set(baseShape, new BaseShapeActions(baseShape))
        }
        return this.actions.get(baseShape)
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        this.actions.forEach(action=>{action.execute()})
    }
}

export {EventGraphComponent}