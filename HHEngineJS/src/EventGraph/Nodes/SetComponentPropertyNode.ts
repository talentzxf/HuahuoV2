import {AbstractNode} from "./AbstractNode";
import {INodeOutputSlot, LGraphNode, LiteGraph} from "litegraph.js";
import {getComponentProperties} from "../LGraphSetup";
import {PropertyDef} from "../../Components/PropertySheetBuilder";


class SetComponentPropertyNode extends AbstractNode{
    title = "SetComponentProperty"
    desc = "Set component property"

    componentInputSlot

    componentProperties

    constructor() {
        super();

        this.componentInputSlot = this.addInput("component", "component")
    }

    onConnectInput(inputIndex: number, outputType: INodeOutputSlot["type"], outputSlot: INodeOutputSlot, outputNode: LGraphNode, outputIndex: number): boolean {
        if(this.componentInputSlot == inputIndex){
            let componentType = outputNode.properties["componentType"]
            this.refreshComponentProperties(componentType)
        }
    }

    refreshComponentProperties(componentType: string){
        this.componentProperties = getComponentProperties(componentType)

        let allPropertyNames = []
        this.componentProperties.forEach((propertyEntry: PropertyDef)=>{
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