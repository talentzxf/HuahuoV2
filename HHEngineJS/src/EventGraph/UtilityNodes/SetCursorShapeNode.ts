import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js"
import {renderEngine2D} from "../../index";

class SetCursorShapeNode extends AbstractNode {

    constructor() {
        super();

        this.addInput("Execute", LiteGraph.EVENT)
        this.addInput("cursorShape", "string")
    }

    getTitle(): string {
        return "SetCursorShape"
    }

    onAction(action, param) {
        let cursorShape = this.getInputDataByName("cursorShape", true)
        renderEngine2D.getDefaultCanvas().style.cursor = cursorShape
    }

    static getType() {
        return "cursor/set-shape"
    }
}

LiteGraph.registerNodeType(SetCursorShapeNode.getType(), SetCursorShapeNode)

export {SetCursorShapeNode};