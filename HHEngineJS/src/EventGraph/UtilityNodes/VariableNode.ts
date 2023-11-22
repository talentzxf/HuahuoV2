import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";

class VariableNode extends AbstractNode {
    title = "Variable Node"
    desc = "Stores/Reads variable value"

    constructor() {
        super();

        this.addInput("SetValue", LiteGraph.EVENT)
        // @ts-ignore
        this.addInput("input", 0)
        this.addOutput("Executed", LiteGraph.EVENT)
        // @ts-ignore
        this.addOutput("output", 0)
    }

    onAction(action, param) {
        let inputObject = this.getInputDataByName("input", true)
        this.setOutputData(1, inputObject)

        this.triggerSlot(0, null, null)
    }

    static getType(): string {
        return "logic/variable"
    }
}

LiteGraph.registerNodeType(VariableNode.getType(), VariableNode)
export {VariableNode}