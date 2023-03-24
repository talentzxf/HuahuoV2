import {LGraphNode, LiteGraph} from "litegraph.js";
import {eventBus} from "hhcommoncomponents";

class EventNode extends LGraphNode {
    title = "EventNode"
    desc = "Triggers if event happens"
    fullEventName = null

    constructor() {
        super();
        this.addOutput("Execute", LiteGraph.EVENT)
    }

    setFullEventName(fullEventName: string) {
        if (this.fullEventName) {
            // TODO: Unregister previously registered events.
        } else {
            let eventNameMeta = eventBus.splitFullEventName(fullEventName)

            this.fullEventName = fullEventName
            this.title = fullEventName

            let _this = this
            eventBus.addEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, () => {
                _this.triggerSlot(0, null, null)
            })
        }

    }
}

LiteGraph.registerNodeType("events/eventNode", EventNode)

export {EventNode}