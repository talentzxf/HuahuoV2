import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseShapeEvents} from "../EventGraph/BaseShapeEvents";
import {LGraph} from "litegraph.js";

@Component()
class EventGraphComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.stringValue, null, null, true)
    eventGraphJSON

    // This corresponds to the button editing the event graph.
    @PropertyValue(PropertyCategory.customField)
    eventGraph

    graph: LGraph

    // TODO: Need persist of following arrays.
    actions:Map<BaseShapeJS, BaseShapeActions> = new Map

    eventEmitters: Map<BaseShapeJS, BaseShapeEvents> = new Map

    getGraph(){
        if(this.graph == null)
            this.graph = new LGraph()
        return this.graph
    }

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