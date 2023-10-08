import {LiteGraph, SerializedLGraphNode} from "litegraph.js";
import {i18n, splitFullEventName} from "hhcommoncomponents";
import {AbstractNode} from "./AbstractNode";
import {huahuoEngine} from "../../EngineAPI";

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

    eventHasBeenSet: boolean = false

    unsubscribeEvent(){
        let fullEventName = this.properties.fullEventName
        let eventNameMeta = splitFullEventName(fullEventName)

        let targetEventBus = this.getEventGraphComponent().getEventBusForNode(this.id)
        if (this.properties.fullEventName && this.currentEventHandler > 0) {
            targetEventBus.removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, this.currentEventHandler)
        }
    }

    reset() {
        this.eventHasBeenSet = false
        this.unsubscribeEvent()
    }

    onExecute() {
        if (!this.eventHasBeenSet) {
            this.setupEvent()

            this.eventHasBeenSet = true
        }
    }

    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.getParamMap()[paramIdx] = outputSlot
    }

    getParamMap() {
        return this.properties.paramIdxOutputSlotMap
    }

    setFullEventName(fullEventName: string, title = null) {
        this.properties.fullEventName = fullEventName
        this.title = title || fullEventName
    }

    // TODO: The event bus might not be the global one.
    setupEvent() {
        this.unsubscribeEvent()

        let fullEventName = this.properties.fullEventName
        let eventNameMeta = splitFullEventName(fullEventName)

        let targetEventBus = this.getEventGraphComponent().getEventBusForNode(this.id)

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