import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";

let defaultNodeName = "Default"
let executedNodeName = "Executed"
let retainNodes = [defaultNodeName, executedNodeName]

class SwitchNode extends AbstractNode {
    title = "Switch Node"
    desc = "Switch case, execute under different conditions"

    inputWidget

    properties = {
        options: {}
    }

    syncValue(optionArrayValue) {
        // Remove all outputs that's not in the value.
        let outputNodes = this.outputs;
        if (outputNodes != null && outputNodes.length > 0) {
            for (let output of outputNodes) {
                if (retainNodes.includes(output.name)) {
                    continue
                }

                if (!optionArrayValue.includes(output.name)) {
                    let slotId = this.findOutputSlot(output.name);
                    this.removeOutput(slotId);
                }
            }
        }

        for (let val of optionArrayValue) {
            let slotId = this.findOutputSlot(val);
            if (slotId == -1) {
                this.addOutput(val, LiteGraph.EVENT);
            }
        }

        this.graph.afterChange(this);
    }

    properties_info = [{
        name: "options",
        type: "array",
        allowEmpty: false,
        unique: true,
        callback: (name, arrayValue, options) => {
            this.syncValue(arrayValue)
        }
    }]

    constructor() {
        super();
        this.addInput("Execute", LiteGraph.EVENT)
        this.addOutput(executedNodeName, LiteGraph.EVENT)

        // If none matched, execute this slot.
        this.addOutput(defaultNodeName, LiteGraph.EVENT)
        this.inputWidget = this.addInput("input", "string")
    }

    onAction(action, param) {
        let inputString = this.getInputDataByName("input")
        let executed = false
        for (let output of this.outputs) {
            if (output.name == inputString) {
                executed = true

                let slotId = this.findOutputSlot(output.name)
                this.triggerSlot(slotId, null, null)
            }
        }

        if (!executed) {
            let defaultSlot = this.findOutputSlot(defaultNodeName)
            this.triggerSlot(defaultSlot, null, null)
        }

        this.triggerSlot(0, null, null)
    }

    static getType(): string {
        return "logic/switch"
    }
}

LiteGraph.registerNodeType(SwitchNode.getType(), SwitchNode)

export {SwitchNode}