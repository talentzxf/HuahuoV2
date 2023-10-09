import {LiteGraph} from "litegraph.js";
import {GetShapeComponentNode} from "./Nodes/GetShapeComponentNode";
import {PropertyDef} from "../Components/PropertySheetBuilder";
import {SetComponentPropertyNode} from "./Nodes/SetComponentPropertyNode";
import {ActionNode} from "./Nodes/ActionNode";

let componentNamePropertiesMap: Map<string, Array<PropertyDef>> = new Map<string, Array<PropertyDef>>()

function addComponentProperties(componentName: string, properties: Array<PropertyDef>) {
    componentNamePropertiesMap.set(componentName, properties)
}

function getComponentProperties(componentName: string): Array<PropertyDef> {
    return componentNamePropertiesMap.get(componentName)
}

function setupLGraph() {
    // @ts-ignore
    LiteGraph.slot_types_default_out["shape"] = [GetShapeComponentNode.getType()]

    // @ts-ignore
    LiteGraph.slot_types_default_out["component"] = [SetComponentPropertyNode.getType()]

    // @ts-ignore
    LiteGraph.slot_types_default_out["_event_"] = [ActionNode.getType()].concat(LiteGraph.slot_types_default_out["_event_"]).filter((v) => {
        return v ? true : false
    })
}

export {setupLGraph, addComponentProperties, getComponentProperties}