import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";

let defaultNodeName = "Default"

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
                if (output.name == defaultNodeName) {
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

        // If none matched, execute this slot.
        this.addOutput(defaultNodeName, LiteGraph.EVENT)
        this.inputWidget = this.addInput("input", "string")
    }

    static getType(): string {
        return "logic/switch"
    }
}

LiteGraph.registerNodeType(SwitchNode.getType(), SwitchNode)

export {SwitchNode}