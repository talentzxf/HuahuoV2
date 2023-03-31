import {LiteGraph, SerializedLGraphNode} from "litegraph.js";
import {splitFullEventName} from "HHCommonComponents";
import {AbstractNode} from "./AbstractNode";

class EventNode extends AbstractNode {
    title = "EventNode"
    desc = "Triggers if event happens"

    properties = {
        fullEventName: null,
        paramIdxOutputSlotMap: null
    }

    currentEventHandler = -1

    constructor() {
        super();
        this.addOutput("Execute", LiteGraph.EVENT)
    }

    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.getParamMap().set(paramIdx, outputSlot)
    }

    getParamMap() {
        if (this.properties.paramIdxOutputSlotMap == null) {
            this.properties.paramIdxOutputSlotMap = new Map
        }
        return this.properties.paramIdxOutputSlotMap
    }

    // TODO: The event bus might not be the global one.
    setupEvent(fullEventName: string) {

        let targetEventBus = this.getEventGraphComponent().getEventBus(this.id)

        let eventNameMeta = splitFullEventName(fullEventName)
        if (this.properties.fullEventName && this.currentEventHandler > 0) {
            targetEventBus.removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, this.currentEventHandler)
        }

        this.properties.fullEventName = fullEventName
        this.title = fullEventName
        let _this = this
        this.currentEventHandler = targetEventBus.addEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, (params) => {
            console.log(params)
            for (let paramIdx = 0; paramIdx < params.length; paramIdx++) {
                let slot = _this.getParamMap().get(paramIdx)
                if (slot) {
                    let slotIdx = _this.findOutputSlot(slot.name)
                    if (slotIdx >= 0) {
                        _this.setOutputData(slotIdx, params[paramIdx]);
                    }
                }
            }
            _this.triggerSlot(0, null, null)
            _this.setDirtyCanvas(true, true)
            _this.graph.afterChange()
        })
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