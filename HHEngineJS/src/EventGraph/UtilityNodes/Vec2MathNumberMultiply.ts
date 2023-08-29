import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";
import {convertGraphValueToComponentValue} from "../GraphUtils";

class Vec2MathNumberMultiply extends AbstractNode {
    values = ["*", "/"]

    slotA
    slotB

    slotResult

    _func: Function

    constructor() {
        super();

        this.slotA = this.addInput("vector A", "vec2")
        this.slotB = this.addInput("number B", "number")

        this.slotResult = this.addOutput("vector C", "vec2")

        this.addProperty("OP", "*", "enum", {
            values: this.values
        })
    }

    getTitle(): string {
        return "A " + this.properties.OP + " B"
    }

    onExecute() {
        let A = convertGraphValueToComponentValue(this.getInputData(0), "vec2")
        let B = this.getInputData(1)

        if(A == null || B == null)
            return

        let res = {
            x: 0.0,
            y: 0.0
        }

        switch (this.properties.OP) {
            case "*":
                res.x = A.x * B
                res.y = A.y * B
                break;
            case "-":
                res.x = A.x / B
                res.y = A.y / B
                break;
        }

        this.setOutputData(0, res)
    }

    static getType() {
        return "math2d/number-vector-operator"
    }
}


LiteGraph.registerNodeType(Vec2MathNumberMultiply.getType(), Vec2MathNumberMultiply)

export {Vec2MathNumberMultiply}