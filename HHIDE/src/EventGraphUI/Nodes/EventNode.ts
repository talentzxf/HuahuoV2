import {LGraphNode, LiteGraph} from "litegraph.js";
import {eventBus} from "hhcommoncomponents";

class EventNode extends LGraphNode {
    title = "EventNode"
    desc = "Triggers if event happens"
    fullEventName = null

    paramIdxOutputSlotMap = new Map

    constructor() {
        super();
        this.addOutput("Execute", LiteGraph.EVENT)
    }

    addParameterIndexSlotMap(paramIdx, outputSlot){
        this.paramIdxOutputSlotMap.set(paramIdx, outputSlot)
    }

    setFullEventName(fullEventName: string) {
        if (this.fullEventName) {
            // TODO: Unregister previously registered events.
        } else {
            let eventNameMeta = eventBus.splitFullEventName(fullEventName)

            this.fullEventName = fullEventName
            this.title = fullEventName

            let _this = this
            eventBus.addEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, (params) => {
                console.log(params)
                for(let paramIdx = 0; paramIdx < params.length; paramIdx++){
                    let slot = _this.paramIdxOutputSlotMap.get(paramIdx)
                    if(slot){
                        _this.setOutputData(slot.slot_index, params[paramIdx]);
                    }
                }
                _this.triggerSlot(0, null, null)
                _this.setDirtyCanvas(true, true)
                _this.graph.afterChange()
            })
        }

    }
}

LiteGraph.registerNodeType("events/eventNode", EventNode)

export {EventNode}