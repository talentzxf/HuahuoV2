import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";
import {ActionNode} from "../Nodes/ActionNode";

class Vector2NumberNode extends AbstractNode{
    title = "Vector2Number"
    desc = "Split Vector2 into x and y"

    constructor() {
        super();
        this.addInput("in", "vec2");
        this.addOutput("x", "number");
        this.addOutput("y", "number");
    }

    onExecute() {
        let v = this.getInputData(0)
        if(v == null)
            return;

        this.setOutputData(0, v.x)
        this.setOutputData(1, v.y)
    }

    static getType(): string {
        return "math/vector2number"
    }

}

//LiteGraph.registerNodeType(Vector2NumberNode.getType(), Vector2NumberNode)
export {Vector2NumberNode}