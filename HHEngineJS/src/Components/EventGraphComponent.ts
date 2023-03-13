import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

@Component()
class EventGraphComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.stringValue, {random: true})
    eventGraphJSON

    @PropertyValue(PropertyCategory.customField)
    editEventGraph

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);
    }
}

export {EventGraphComponent}