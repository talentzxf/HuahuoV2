import {AbstractNode} from "../Nodes/AbstractNode";
import {convertGraphValueToComponentValue} from "../GraphUtils";
import {LiteGraph} from "litegraph.js";

class Vec2MathOperationNode extends AbstractNode {
    values = ["+", "-"]

    slotA
    slotB

    slotResult

    _func: Function

    constructor() {
        super();

        this.slotA = this.addInput("A", "vec2")
        this.slotB = this.addInput("B", "vec2")

        this.slotResult = this.addOutput("C", "vec2")

        this.addProperty("OP", "+", "enum", {
            values: this.values
        })
    }

    getTitle(): string {
        return "A " + this.properties.OP + " B"
    }

    onExecute() {
        let A = convertGraphValueToComponentValue(this.getInputData(0), "vec2")
        let B = convertGraphValueToComponentValue(this.getInputData(1), "vec2")

        if(A == null || B == null)
            return

        let res = {
            x: 0.0,
            y: 0.0
        }

        switch (this.properties.OP) {
            case "+":
                res.x = A.x + B.x
                res.y = A.y + B.y
                break;
            case "-":
                res.x = A.x - B.x
                res.y = A.y - B.y
                break;
        }

        this.setOutputData(0, res)
    }

    static getType(){
        return "math2d/operator"
    }
}


LiteGraph.registerNodeType(Vec2MathOperationNode.getType(), Vec2MathOperationNode)

export {Vec2MathOperationNode}