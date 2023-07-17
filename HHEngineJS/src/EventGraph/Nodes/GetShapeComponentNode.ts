import {AbstractNode} from "./AbstractNode";
import {LiteGraph} from "litegraph.js";
import {huahuoEngine} from "../../EngineAPI";

class GetShapeComponentNode extends AbstractNode{
    title = "GetShapeComponentNode"
    desc = "Get component from shape by component type"

    constructor() {
        super();

        let allComponentNames = huahuoEngine.getAllCompatibleComponents(new Object())

        this.addProperty("componentType", null, "enum", {
            values: allComponentNames
        })

        this.addInput("shape", "shape")
        this.addOutput("component", "component")
    }

    static getType(): string{
        return "shape/getComponent"
    }
}

LiteGraph.registerNodeType(GetShapeComponentNode.getType(), GetShapeComponentNode)

export {GetShapeComponentNode}