import {LGraphNode, LiteGraph} from "litegraph.js";

class ActionNode extends LGraphNode {
    title = "ActionNode"

    actionTarget

    executedSlot
    constructor() {
        super();
        this.addInput("Execute", LiteGraph.EVENT)
        this.executedSlot = this.addOutput("Executed", LiteGraph.EVENT)
    }

    setActionTarget(actionTarget){
        this.actionTarget = actionTarget
    }

    setActionName(actionName){
        this.title = actionName
    }

    onAction(action, param){
        console.log("Something happened!")

        // Trigger output slot
        this.triggerSlot(this.executedSlot.slot_index, null, null)
    }

    paramIdxOutputSlotMap = new Map
    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.paramIdxOutputSlotMap.set(paramIdx, outputSlot)
    }
}

LiteGraph.registerNodeType("actions/actionNode", ActionNode)
export {ActionNode}