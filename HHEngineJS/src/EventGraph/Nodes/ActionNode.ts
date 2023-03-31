import {LiteGraph} from "litegraph.js";
import {AbstractNode} from "./AbstractNode";

class ActionNode extends AbstractNode {
    title = "ActionNode"

    actionName = null

    executedSlot


    constructor() {
        super();
        this.addInput("Execute", LiteGraph.EVENT)
        this.executedSlot = this.addOutput("Executed", LiteGraph.EVENT)
    }

    setActionName(actionName) {
        this.title = actionName
        this.actionName = actionName
    }

    onAction(action, param) {
        console.log("Something happened!")

        let callBackParams = []

        for (let paramIdx = 0; paramIdx <= this.maxParamIdx; paramIdx++) {
            let slot = this.paramIdxSlotMap.get(paramIdx)
            if (slot) {
                let inputData = this.getInputDataByName(slot.name)
                callBackParams.push(inputData)
            } else {
                callBackParams.push(null)
            }
        }

        let actionTarget = this.getEventGraphComponent().getActionTarget(this.id)

        let func = actionTarget[this.actionName]
        if (func)
            func.apply(actionTarget, callBackParams)

        let executedSlotId = this.findOutputSlot(this.executedSlot.name)
        if (executedSlotId >= 0) {
            // Trigger output slot
            this.triggerSlot(executedSlotId, null, null)
        }
    }

    maxParamIdx = -1 // -1 means no parameter.
    paramIdxSlotMap = new Map

    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.paramIdxSlotMap.set(paramIdx, outputSlot)
        if (paramIdx > this.maxParamIdx) {
            this.maxParamIdx = paramIdx
        }
    }

    static getType(): string {
        return "actions/actionNode"
    }
}

LiteGraph.registerNodeType(ActionNode.getType(), ActionNode)
export {ActionNode}