import {LGraphNode, LiteGraph} from "litegraph.js";
import {eventBus} from "HHCommonComponents";

class EventNode extends LGraphNode {
    title = "EventNode"
    desc = "Triggers if event happens"
    fullEventName = null

    paramIdxOutputSlotMap = new Map

    currentEventHandler = -1

    constructor() {
        super();
        this.addOutput("Execute", LiteGraph.EVENT)
    }

    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.paramIdxOutputSlotMap.set(paramIdx, outputSlot)
    }

    setFullEventName(fullEventName: string) {
        let eventNameMeta = eventBus.splitFullEventName(fullEventName)
        if (this.fullEventName && this.currentEventHandler > 0) {
            eventBus.removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, this.currentEventHandler)
        }

        this.fullEventName = fullEventName
        this.title = fullEventName
        let _this = this
        this.currentEventHandler = eventBus.addEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, (params) => {
            console.log(params)
            for (let paramIdx = 0; paramIdx < params.length; paramIdx++) {
                let slot = _this.paramIdxOutputSlotMap.get(paramIdx)
                if (slot) {
                    let slotIdx = _this.findOutputSlot(slot.name)
                    if(slotIdx >= 0){
                        _this.setOutputData(slotIdx, params[paramIdx]);
                    }
                }
            }
            _this.triggerSlot(0, null, null)
            _this.setDirtyCanvas(true, true)
            _this.graph.afterChange()
        })
    }
}

LiteGraph.registerNodeType("events/eventNode", EventNode)

export {EventNode}