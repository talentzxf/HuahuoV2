import {LGraph} from "litegraph.js";
import {AbstractGraphAction, ActionDef, ActionParam, GraphAction} from "./GraphActions";
import {EventEmitter, GraphEvent, PropertyType} from "hhcommoncomponents";
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
        if (isGlobal) {
            huahuoEngine.getActivePlayer().setFrameId(frameId)
        } else {
            this.layer.SetCurrentFrame(frameId)
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

    linkNodeWithTarget(id: number, sourceObj) {
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
}

export {LayerGraphWrapper}