import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph, SerializedLGraphNode} from "litegraph.js";

class ConstVec2Node extends AbstractNode {
    title = "Const Vector2"
    desc = "Create a const vec2 node"

    xWidget
    yWidget

    properties = {
        x: 1,
        y: 1
    }

    constructor() {
        super();
        let _this = this
        this.xWidget = this.addWidget("number", "valueX", this.properties.x, (v) => {
            _this.properties.x = v
            _this.graph.afterChange(_this)

        })
        this.yWidget = this.addWidget("number", "valueY", this.properties.y, (v) => {
            _this.properties.y = v
            _this.graph.afterChange(_this)
        })
        this.addOutput("value", "vec2");
        this.widgets_up = true;
        this.size = [180, 60]
    }

    onConfigure(o: SerializedLGraphNode) {
        if (o.properties.hasOwnProperty("x"))
            this.xWidget.value = o.properties.x

        if (o.properties.hasOwnProperty("y"))
            this.yWidget.value = o.properties.y
    }

    onExecute() {
        this.properties.x = this.xWidget.value
        this.properties.y = this.yWidget.value

        this.setOutputData(0, {
            x: this.xWidget.value,
            y: this.yWidget.value
        })
    }

    static getType(): string {
        return "math/constVec2"
    }
}

LiteGraph.registerNodeType(ConstVec2Node.getType(), ConstVec2Node)

export {ConstVec2Node}