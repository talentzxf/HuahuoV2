import {LGraphNode, LiteGraph} from "litegraph.js";

class ActionNode extends LGraphNode {
    title = "ActionNode"

    constructor() {
        super();
        this.addInput("Execute", LiteGraph.EVENT)
        this.addOutput("Executed", LiteGraph.EVENT)
    }

    setActionName(actionName){
        this.title = actionName
    }

    paramIdxOutputSlotMap = new Map
    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.paramIdxOutputSlotMap.set(paramIdx, outputSlot)
    }
}

LiteGraph.registerNodeType("actions/actionNode", ActionNode)
export {ActionNode}