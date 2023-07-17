import {LiteGraph} from "litegraph.js";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {GetShapeComponentNode} from "./Nodes/GetShapeComponentNode";
import {PropertyDef} from "../Components/PropertySheetBuilder";
import {SetComponentPropertyNode} from "./Nodes/SetComponentPropertyNode";

let componentNamePropertiesMap:Map<string, Array<PropertyDef>> = new Map<string, Array<PropertyDef>>()
function addComponentProperties(componentName: string, properties: Array<PropertyDef>){
    componentNamePropertiesMap.set(componentName, properties)
}

function getComponentProperties(componentName: string):Array<PropertyDef>{
    return componentNamePropertiesMap.get(componentName)
}

function setupLGraph(){
    // @ts-ignore
    LiteGraph.slot_types_default_out["shape"] = [GetShapeComponentNode.getType()]

    // @ts-ignore
    LiteGraph.slot_types_default_out["component"] = [SetComponentPropertyNode.getType()]
}

export {setupLGraph, addComponentProperties, getComponentProperties}