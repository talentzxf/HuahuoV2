import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

@Component()
class EventGraphComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.stringValue, null, null, true)
    eventGraphJSON

    @PropertyValue(PropertyCategory.customField)
    eventGraph

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);
    }
}

export {EventGraphComponent}