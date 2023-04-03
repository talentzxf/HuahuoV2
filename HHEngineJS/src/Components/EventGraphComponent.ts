import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {LGraph} from "litegraph.js";
import {huahuoEngine} from "../EngineAPI";
import {eventBus} from "hhcommoncomponents";
import {EventNode} from "../EventGraph/Nodes/EventNode";
import {ActionNode} from "../EventGraph/Nodes/ActionNode";

@Component({compatibleShapes: ["BaseShapeJS"], cppClassName: "EventGraphComponent"})
class EventGraphComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", null, true)
    eventGraphJSON

    // This corresponds to the button editing the event graph.
    @PropertyValue(PropertyCategory.customField)
    eventGraph

    graph: LGraph

    // If shape is null, this node is listening to global event.
    linkNodeWithTarget(nodeId: number, shape: BaseShapeJS){
        if(shape != null)
            this.rawObj.AddNodeIdShapeMap(nodeId, shape.getRawShape())
        else
            this.rawObj.AddNodeIdShapeMap(nodeId, null)
    }

    getActionTarget(nodeId: number){
        let rawObj = this.rawObj.GetShapeByNodeId(nodeId)
        let baseShapeObj = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(rawObj)

        return baseShapeObj.getAction()
    }

    getEventBus(nodeId: number){
        let rawObj = this.rawObj.GetShapeByNodeId(nodeId)
        if(rawObj == null)
            return eventBus

        let baseShapeObj = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(rawObj)
        return huahuoEngine.getEvent(baseShapeObj).getEventBus()
    }

    saveGraph() {
        let graphString = JSON.stringify(this.graph.serialize())
        if (this.eventGraphJSON != graphString)
            this.eventGraphJSON = graphString
    }

    constructor(rawObj?) {
        let needLoad = rawObj ? true : false;
        super(rawObj);

        this.graph = new LGraph()
        if (this.eventGraphJSON && this.eventGraphJSON.length > 0) {
            let data = JSON.parse(this.eventGraphJSON)
            this.graph.configure(data)
        }

        this.graph.start()
        this.graph["onAfterChange"] = this.saveGraph.bind(this)

        if(needLoad){
            let eventNodes = this.graph.findNodesByType(EventNode.getType())
            for(let node of eventNodes){
                let eventNode = node as EventNode
                eventNode.setEventGraphComponent(this)
                eventNode.setupEvent(eventNode.properties.fullEventName)
            }

            let actionNodes = this.graph.findNodesByType(ActionNode.getType())
            for(let actionNode of actionNodes){
                (actionNode as ActionNode).setEventGraphComponent(this)
            }
        }
    }

    getGraph() {
        return this.graph
    }
}

export {EventGraphComponent}