import {AbstractNode} from "./AbstractNode";
import {INodeOutputSlot, LGraphNode, LiteGraph} from "litegraph.js";
import {getComponentProperties} from "../LGraphSetup";
import {PropertyDef} from "../../Components/PropertySheetBuilder";


class SetComponentPropertyNode extends AbstractNode{
    title = "SetComponentProperty"
    desc = "Set component property"

    componentInputSlot

    componentProperties

    executeSlot
    executedSlot

    constructor() {
        super();

        this.executeSlot = this.addInput("Execute", LiteGraph.EVENT)
        this.executedSlot = this.addOutput("Executed", LiteGraph.EVENT)

        this.componentInputSlot = this.addInput("component", "component")
    }



    onConnectInput(inputIndex: number, outputType: INodeOutputSlot["type"], outputSlot: INodeOutputSlot, outputNode: LGraphNode, outputIndex: number): boolean {
        if(this.componentInputSlot == this.inputs[inputIndex]){
            let componentType = outputNode.properties["componentType"]
            this.refreshComponentProperties(componentType)

            let executedSlot = outputNode.findOutputSlot("Executed")
            if (executedSlot >= 0) {
                outputNode.connect(executedSlot, this, "Execute")
            }
        }
        return true
    }

    refreshComponentProperties(componentType: string){
        this.componentProperties = getComponentProperties(componentType)

        let allPropertyNames = []
        this.componentProperties?.forEach((propertyEntry: PropertyDef)=>{
            allPropertyNames.push(propertyEntry.key)
        })

        this.properties_info = this.properties_info.filter((property_info)=>{
            return property_info.name != "propertyName"
        })

        this.addProperty("propertyName", null, "enum", {
            values: allPropertyNames
        })
    }

    static getType(): string{
        return "component/setProperty"
    }
}

LiteGraph.registerNodeType(SetComponentPropertyNode.getType(), SetComponentPropertyNode)

export {SetComponentPropertyNode}