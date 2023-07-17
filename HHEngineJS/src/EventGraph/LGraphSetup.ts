import {LiteGraph} from "litegraph.js";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {GetShapeComponentNode} from "./Nodes/GetShapeComponentNode";

function getComponentByTypeName(shape: BaseShapeJS, componentName:string){
    return shape.getComponentByTypeName(componentName)
}

function setupLGraph(){
    // @ts-ignore
    LiteGraph.slot_types_default_out["shape"] = [GetShapeComponentNode.getType()]
}

export {setupLGraph}