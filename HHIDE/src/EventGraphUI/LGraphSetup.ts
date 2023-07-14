import {LiteGraph} from "hhenginejs";
import {BaseShapeJS} from "hhenginejs";

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
        ["component"]
    )
    LiteGraph.slot_types_default_out["shape"] = ["shape/getComponentByTypeName"]
}

export {setupLGraph}