import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";

class SwitchNode extends AbstractNode {
    title = "Switch Node"
    desc = "Switch case, execute under different conditions"

    inputWidget

    constructor() {
        super();
        this.inputWidget = this.addInput("input", "string")
    }

    static getType(): string {
        return "logic/switch"
    }
}

LiteGraph.registerNodeType(SwitchNode.getType(), SwitchNode)

export {SwitchNode}