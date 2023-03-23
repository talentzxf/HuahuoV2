import {LGraphNode, LiteGraph} from "litegraph.js";

class EventNode extends LGraphNode{
    title = "EventNode"
    desc = "Triggers if event happens"

    constructor() {
        super();
        this.addOutput("Execute", LiteGraph.EVENT)
    }

    onExecute() {
        this.triggerSlot(0, null, null)
    }
}

LiteGraph.registerNodeType("events/eventNode", EventNode)

export {EventNode}