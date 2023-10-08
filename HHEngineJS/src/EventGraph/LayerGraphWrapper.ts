import {LGraph} from "litegraph.js";
import {AbstractGraphAction, ActionDef, ActionParam, GraphAction, NodeTargetType} from "./GraphActions";
import {eventBus, EventEmitter, GraphEvent, PropertyType} from "hhcommoncomponents";
import {layerUtils} from "../LayerUtils";
import {huahuoEngine} from "../EngineAPI";

class LayerFrameActor extends AbstractGraphAction {
    layer

    constructor(layer) {
        super();
        this.layer = layer
    }

    @GraphAction(true)
    setFrameId(@ActionParam(PropertyType.NUMBER) frameId: number, @ActionParam(PropertyType.BOOLEAN) isGlobal: boolean = false) {
        if (frameId <= 0) {
            console.error("Can't set less than 1 frameId")
            return
        }

        let actualFrameId = frameId - 1

        if (isGlobal) {
            huahuoEngine.getActivePlayer().setFrameId(actualFrameId)
        } else {
            this.layer.SetCurrentFrame(actualFrameId)
        }
    }
}

class LayerGraphWrapper extends EventEmitter {
    graphParams

    graph

    layerActor

    constructor(graphParams) {
        super()
        this.graphParams = graphParams

        this.graph = new LGraph()

        let graphJson = graphParams?.GetEventGraphJSON()
        if (graphJson && graphJson.length > 0) {
            let data = JSON.parse(graphJson)
            this.graph.configure(data)
        }

        this.graph["onAfterChange"] = this.saveGraph.bind(this)

        let layer = this.graphParams.GetLayer()
        let frameId = this.graphParams.GetFrameId()

        layerUtils.addPlayFrameCallbacks(layer, frameId, this.onPlayFrame.bind(this))

        this.layerActor = new LayerFrameActor(layer)
    }

    // TODO: This should be persisted.zhi
    nodeIdTargetMap = new Map()
    nodeTargetTypeMap = new Map()

    linkNodeWithTarget(id: number, targetType: NodeTargetType, sourceObj) {
        this.nodeTargetTypeMap.set(id, targetType)
        this.nodeIdTargetMap.set(id, sourceObj)
    }

    getActionTarget(id: number) {
        return this.nodeIdTargetMap.get(id)
    }

    getBaseActor() {
        return this.layerActor
    }

    @GraphEvent(true)
    onPlayFrame() {
    }

    getGraph() {
        return this.graph
    }

    saveGraph() {
        let graphString = JSON.stringify(this.graph.serialize())
        this.graphParams.SetEventGraphJSON(graphString)
    }

    getActionDefs(): Array<ActionDef> {
        return undefined;
    }

    getEventBusForNode(nodeId: number) {
        let eventSource = this.nodeIdTargetMap.get(nodeId)
        if(eventSource == null){
            return eventBus
        }

        return eventSource.getEventBus()
    }
}

export {LayerGraphWrapper}