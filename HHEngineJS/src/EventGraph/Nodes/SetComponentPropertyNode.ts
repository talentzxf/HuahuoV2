import {AbstractNode} from "./AbstractNode";
import {INodeOutputSlot, LGraphNode, LiteGraph} from "litegraph.js";
import {getComponentProperties} from "../LGraphSetup";
import {PropertyDef} from "../../Components/PropertySheetBuilder";
import {getLiteGraphTypeFromPropertyCategory} from "../GraphUtils";

class SetComponentPropertyNode extends AbstractNode {
    title = "SetComponentProperty"
    desc = "Set component property"

    componentInputSlot

    componentProperties

    inputParameterSlot = null

    executeSlot
    executedSlot

    constructor() {
        super();

        this.executeSlot = this.addInput("Execute", LiteGraph.EVENT)
        this.executedSlot = this.addOutput("Executed", LiteGraph.EVENT)

        this.componentInputSlot = this.addInput("component", "component")
    }

    onAction(action, param) {
        let componentInputSlotIdx = this.findInputSlot(this.componentInputSlot.name)
        let inputComponent = this.getInputData(componentInputSlotIdx)

        let inputParameterSlotIdx = this.findInputSlot(this.inputParameterSlot.name)
        let inputParameterValue = this.getInputData(inputParameterSlotIdx, true)
        inputComponent[this.inputParameterSlot.name] = inputParameterValue
    }

    onConnectInput(inputIndex: number, outputType: INodeOutputSlot["type"], outputSlot: INodeOutputSlot, outputNode: LGraphNode, outputIndex: number): boolean {
        if (this.componentInputSlot == this.inputs[inputIndex]) {
            let componentType = outputNode.properties["componentType"]
            this.refreshComponentProperties(componentType)

            let executedSlot = outputNode.findOutputSlot("Executed")
            if (executedSlot >= 0) {
                outputNode.connect(executedSlot, this, "Execute")
            }
        }
        return true
    }

    onPropertyChanged(property: string, value: any, prevValue: any): void | boolean {
        if (property == "propertyName") {
            let propertyName = value
            let properties = this.componentProperties.filter((property) => {
                return property.key == propertyName
            })

            if (properties.length != 1) {
                console.error("Should have one and only one property:" + propertyName)
            } else {
                let propertyEntry = properties[0]
                let graphType = getLiteGraphTypeFromPropertyCategory(propertyEntry.type)

                if(this.inputParameterSlot == null){
                    this.inputParameterSlot = this.addInput(propertyName, graphType)
                }else{
                    this.inputParameterSlot.name = propertyName
                    this.inputParameterSlot.type = graphType
                }

            }
        }
    }

    refreshComponentProperties(componentType: string) {
        this.componentProperties = getComponentProperties(componentType)

        let allPropertyNames = []
        this.componentProperties?.forEach((propertyEntry: PropertyDef) => {
            allPropertyNames.push(propertyEntry.key)
        })

        this.properties_info = this.properties_info.filter((property_info) => {
            return property_info.name != "propertyName"
        })

        this.addProperty("propertyName", null, "enum", {
            values: allPropertyNames
        })

        if(this.inputParameterSlot != null){
            this.inputParameterSlot.name = null
            this.inputParameterSlot.type = null
        }
    }

    static getType(): string {
        return "component/setProperty"
    }
}

LiteGraph.registerNodeType(SetComponentPropertyNode.getType(), SetComponentPropertyNode)

export {SetComponentPropertyNode}