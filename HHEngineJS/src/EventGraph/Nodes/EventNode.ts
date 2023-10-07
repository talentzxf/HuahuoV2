import {LiteGraph, SerializedLGraphNode} from "litegraph.js";
import {splitFullEventName} from "hhcommoncomponents";
import {AbstractNode} from "./AbstractNode";
import {huahuoEngine} from "../../EngineAPI";
import {i18n} from "hhcommoncomponents";

class EventNode extends AbstractNode {
    title = "EventNode"
    desc = "Triggers if event happens"

    properties = {
        fullEventName: null,
        paramIdxOutputSlotMap: {}
    }

    currentEventHandler = -1

    constructor() {
        super();
        this.addOutput("Execute", LiteGraph.EVENT, {
            label: i18n.t("eventgraph.Execute")
        })
    }

    onExecute() {
        console.log("On Execute!!!")
    }

    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.getParamMap()[paramIdx] = outputSlot
    }

    getParamMap() {
        return this.properties.paramIdxOutputSlotMap
    }

    // TODO: The event bus might not be the global one.
    setupEvent(fullEventName: string, title = null) {

        let targetEventBus = this.getEventGraphComponent().getEventBusForNode(this.id)

        let eventNameMeta = splitFullEventName(fullEventName)
        if (this.properties.fullEventName && this.currentEventHandler > 0) {
            targetEventBus.removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, this.currentEventHandler)
        }

        this.properties.fullEventName = fullEventName
        this.title = title || fullEventName
        let _this = this
        this.currentEventHandler = targetEventBus.addEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, (params) => {
            if (!huahuoEngine.getActivePlayer().isPlaying) // Do not trigger when player is not playing.
                return

            for (let paramIdx = 0; paramIdx < params.length; paramIdx++) {
                let slot = _this.getParamMap()[paramIdx]
                if (slot) {
                    let slotIdx = _this.findOutputSlot(slot.name)
                    if (slotIdx >= 0) {
                        _this.setOutputData(slotIdx, params[paramIdx]);
                    }
                }
            }
            _this.triggerSlot(0, null, null)
            _this.setDirtyCanvas(true, true)

            // if _this.graph is null, means the node has already been deleted. Need to remove the event listener
            // TODO: This remove should happen when the node is removed!
            if (!_this.graph) {
                targetEventBus.removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, _this.currentEventHandler)
            }
        })
    }

    onRemoved() {
        let targetEventBus = this.getEventGraphComponent().getEventBus(this.id)

        let eventNameMeta = splitFullEventName(this.properties.fullEventName)
        targetEventBus.removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, this.currentEventHandler)
        huahuoEngine.getEventBus().removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, this.currentEventHandler)
    }

    // This function will be called after s/l from file.
    onConfigure(o: SerializedLGraphNode) {
        console.log("on Node configured")

        // this.setupEvent(this.properties.fullEventName)
    }

    static getType(): string {
        return "events/eventNode"
    }
}

LiteGraph.registerNodeType(EventNode.getType(), EventNode)

export {EventNode}