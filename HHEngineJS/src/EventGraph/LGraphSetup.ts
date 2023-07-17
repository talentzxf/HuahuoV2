import {LiteGraph} from "litegraph.js";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

function wrapFunctionAsExecutableNode(){

}

function getComponentByTypeName(shape: BaseShapeJS, componentName:string){
    return shape.getComponentByTypeName(componentName)
}

function setupLGraph(){
    LiteGraph.wrapFunctionAsNode(
        "shape/getComponentByTypeName",
        getComponentByTypeName,
        ["shape", "string"],
        "component"
    )
}

export {setupLGraph}