import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {LGraph} from "litegraph.js";
import {huahuoEngine} from "../EngineAPI";
import {eventBus} from "hhcommoncomponents";

@Component({compatibleShapes: ["BaseShapeJS"], cppClassName: "EventGraphComponent"})
class EventGraphComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", null, true)
    eventGraphJSON

    // This corresponds to the button editing the event graph.
    @PropertyValue(PropertyCategory.customField)
    eventGraph

    actions: Map<BaseShapeJS, BaseShapeActions> = new Map

    graph: LGraph

    // If shape is null, this node is listening to global event.
    linkNodeWithTarget(nodeId: number, shape: BaseShapeJS){
        if(shape != null)
            this.rawObj.AddNodeIdShapeMap(nodeId, shape.getRawShape())
        else
            this.rawObj.AddNodeIdShapeMap(nodeId, null)
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
    }

    getGraph() {
        return this.graph
    }

    getAction(baseShape: BaseShapeJS){
        if(!this.actions.has(baseShape)){
            this.actions.set(baseShape, new BaseShapeActions(baseShape))
        }
        return this.actions.get(baseShape)
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        this.actions.forEach(action => {
            action.execute()
        })
    }
}

export {EventGraphComponent}