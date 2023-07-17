import {AbstractNode} from "./AbstractNode";
import {LiteGraph} from "litegraph.js";
import {huahuoEngine} from "../../EngineAPI";

let titleTemplate = "GetShapeComponent"
class GetShapeComponentNode extends AbstractNode{
    title = titleTemplate + "(null)"
    desc = "Get component from shape by component type"

    inputShapeSlot
    componentOutputSlot
    constructor() {
        super();

        let allComponentNames = huahuoEngine.getAllRegisteredComponents()

        this.addProperty("componentType", null, "enum", {
            values: allComponentNames
        })

        this.inputShapeSlot = this.addInput("shape", "shape")
        this.componentOutputSlot = this.addOutput("component", "component")
    }

    onPropertyChanged(property: string, value: any, prevValue: any): void | boolean {
        if(property == "componentType"){
            this.title = titleTemplate + "(" + value + ")"

            let component_slot_index = this.componentOutputSlot.slot_index

            this.getOutputNodes(component_slot_index).forEach((node)=>{
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