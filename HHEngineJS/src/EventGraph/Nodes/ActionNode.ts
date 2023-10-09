import {LiteGraph} from "litegraph.js";
import {AbstractNode} from "./AbstractNode";
import {ActionDef, NodeTargetType, ReturnValueInfo} from "../GraphActions";
import {huahuoEngine} from "../../EngineAPI";
import {getLiteGraphTypeFromPropertyType} from "../GraphUtils";

class ActionNode extends AbstractNode {
    title = "ActionNode"

    executedSlot

    properties = {
        actionName: "unknownAction",
        paramIdxSlotMap: {},
        maxParamIdx: -1, // -1 means no parameter.
        onlyRunWhenPlaing: false,
        returnValueInfo: null
    }

    funcOutputSlot

    constructor() {
        super();
        this.addInput("Execute", LiteGraph.EVENT)
        this.executedSlot = this.addOutput("Executed", LiteGraph.EVENT)
    }

    setReturnSlot(returnValueInfo: ReturnValueInfo) {
        if (returnValueInfo) {
            this.properties.returnValueInfo = {
                valueName: returnValueInfo.valueName,
                valueType: returnValueInfo.valueType
            }

            if (returnValueInfo != null) {
                let returnValueName = returnValueInfo.valueName
                let returnValueType = returnValueInfo.valueType

                this.funcOutputSlot = this.addOutput(returnValueName, getLiteGraphTypeFromPropertyType(returnValueType))
            }
        }
    }

    setActionDef(actionDef: ActionDef) {
        this.title = actionDef.actionName
        this.properties.actionName = actionDef.actionName
        this.properties.onlyRunWhenPlaing = actionDef.onlyRunWhenPlaing

        this.setReturnSlot(actionDef.returnValueInfo)
    }

    getTargetActor(){
        let targetType = this.properties["targetTypeInfo"].type
        if(targetType == null){
            console.error("Node is not inited???")
            return
        }

        let additionalInfo = this.properties["targetTypeInfo"].additionalInfo

        let baseShape = this.eventGraphComponent.baseShape
        switch(targetType){
            case NodeTargetType.SHAPE:
                baseShape.getActor().AddActionInvoker(this.eventGraphComponent)
                return baseShape.getActor()
            case NodeTargetType.COMPONENT:
                let componentIdx = additionalInfo["componentId"]
                let componentRawObj = baseShape.getRawObject().GetFrameStateByIdx(componentIdx)
                let component = baseShape.getComponentByRawObj(componentRawObj)
                return component.getActor()
            case NodeTargetType.GRAPHCOMPONENT:
                return this.eventGraphComponent.getActor()
            case NodeTargetType.PLAYER:
                return this.eventGraphComponent.playerAction
            default:
                console.warn("Action is not defined!!!")
        }
    }

    onAction(action, param) {
        // Player is not playing and this action should only run when playing. Return.
        if (!huahuoEngine.getActivePlayer().isPlaying && this.properties.onlyRunWhenPlaing)
            return

        console.log("Invoking action node:" + this.properties.actionName)

        let callBackParams = []

        for (let paramIdx = 0; paramIdx <= this.properties.maxParamIdx; paramIdx++) {
            let slot = this.properties.paramIdxSlotMap[paramIdx]
            if (slot) {
                let inputData = this.getInputDataByName(slot.name, true)
                callBackParams.push(inputData)
            } else {
                callBackParams.push(null)
            }
        }

        let actionTarget = this.getTargetActor()

        let func = actionTarget[this.properties.actionName]
        if (func) {
            let functionResult = func.apply(actionTarget, callBackParams)

            if (this.properties.returnValueInfo) {
                let outputSlotIndex = this.findOutputSlot(this.properties.returnValueInfo.valueName)
                this.setOutputData(outputSlotIndex, functionResult)
            }
        }

        let executedSlotId = this.findOutputSlot(this.executedSlot.name)
        if (executedSlotId >= 0) {
            // Trigger output slot
            this.triggerSlot(executedSlotId, null, null)
        }
    }

    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.properties.paramIdxSlotMap[paramIdx] = outputSlot
        if (paramIdx > this.properties.maxParamIdx) {
            this.properties.maxParamIdx = paramIdx
        }
    }

    static getType(): string {
        return "actions/actionNode"
    }
}

LiteGraph.registerNodeType(ActionNode.getType(), ActionNode)
export {ActionNode}