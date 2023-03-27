import {LGraphNode, LiteGraph} from "litegraph.js";

class ActionNode extends LGraphNode {
    title = "ActionNode"

    actionName = null

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
        this.actionName = actionName
    }

    onAction(action, param){
        console.log("Something happened!")

        let callBackParams = []

        for(let paramIdx = 0 ; paramIdx <= this.maxParamIdx; paramIdx++){
            let slot = this.paramIdxSlotMap.get(paramIdx)
            if(slot){
                let inputData = this.getInputDataByName(slot.name)
                callBackParams.push(inputData)
            }else{
                callBackParams.push(null)
            }
        }

        let func = this.actionTarget[this.actionName]
        if(func)
            func.apply(this.actionTarget, callBackParams)

        let executedSlotId = this.findOutputSlot(this.executedSlot.name)
        if( executedSlotId >= 0){
            // Trigger output slot
            this.triggerSlot(executedSlotId, null, null)
        }
    }

    maxParamIdx = -1 // -1 means no parameter.
    paramIdxSlotMap = new Map
    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.paramIdxSlotMap.set(paramIdx, outputSlot)
        if(paramIdx > this.maxParamIdx){
            this.maxParamIdx = paramIdx
        }
    }
}

LiteGraph.registerNodeType("actions/actionNode", ActionNode)
export {ActionNode}