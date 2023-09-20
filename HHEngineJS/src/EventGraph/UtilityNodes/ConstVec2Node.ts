import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";

class ConstVec2Node extends AbstractNode {
    title = "Const Vector2"
    desc = "Create a const vec2 node"

    xWidget
    yWidget

    constructor() {
        super();
        this.xWidget = this.addWidget("number", "valueX", 1, "valueX")
        this.yWidget = this.addWidget("number", "valueY", 1, "valueY")
        this.addOutput("value", "vec2");
        this.widgets_up = true;
        this.size = [180, 60]
    }

    onExecute() {
        this.setOutputData(0, {
            x: this.xWidget.value,
            y: this.yWidget.value
        })
    }

    static getType(): string {
        return "math3d/constVec2"
    }
}

LiteGraph.registerNodeType(ConstVec2Node.getType(), ConstVec2Node)

export {ConstVec2Node}