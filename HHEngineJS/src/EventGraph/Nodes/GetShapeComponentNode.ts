import {AbstractNode} from "./AbstractNode";
import {INodeOutputSlot, LGraphNode, LiteGraph} from "litegraph.js";
import {huahuoEngine} from "../../EngineAPI";

let titleTemplate = "GetShapeComponent"
class GetShapeComponentNode extends AbstractNode{
    title = titleTemplate + "(null)"
    desc = "Get component from shape by component type"

    inputShapeSlot
    componentOutputSlot

    executeSlot
    executedSlot
    constructor() {
        super();

        let allComponentNames = huahuoEngine.getAllRegisteredComponents()

        this.addProperty("componentType", null, "enum", {
            values: allComponentNames
        })

        this.executeSlot = this.addInput("Execute", LiteGraph.EVENT)
        this.executedSlot = this.addOutput("Executed", LiteGraph.EVENT)

        this.inputShapeSlot = this.addInput("shape", "shape")
        this.componentOutputSlot = this.addOutput("component", "component")
    }

    onConnectInput(inputIndex: number, outputType: INodeOutputSlot["type"], outputSlot: INodeOutputSlot, outputNode: LGraphNode, outputIndex: number): boolean {
        let inputShapeIndex = this.findInputSlot(this.inputShapeSlot.name)
        if(inputIndex == inputShapeIndex){
            let executedSlot = outputNode.findOutputSlot("Executed")
            if(executedSlot >= 0){
                outputNode.connect(executedSlot, this, "Execute")
            }
        }
        return true
    }

    onPropertyChanged(property: string, value: any, prevValue: any): void | boolean {
        if(property == "componentType"){
            this.title = titleTemplate + "(" + value + ")"

            let component_slot_index = this.findOutputSlot(this.componentOutputSlot.name)

            this.getOutputNodes(component_slot_index)?.forEach((node)=>{
                if(node["refreshComponentProperties"]){
                    node["refreshComponentProperties"](value)
                }
            })
        }
    }

    static getType(): string{
        return "shape/getComponent"
    }
}

LiteGraph.registerNodeType(GetShapeComponentNode.getType(), GetShapeComponentNode)

export {GetShapeComponentNode}