import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph, SerializedLGraphNode} from "litegraph.js";
import {SwitchNode} from "./SwitchNode";

class ConstStringArrayNode extends AbstractNode{
    title = "String Array"
    desc = "Array of Const Strings"

    properties = {
        contents: {}
    }

    properties_info = [{
        name: "contents",
        type: "array",
        allowEmpty: false,
        unique: true,
        callback: (name, arrayValue, options) => {
            this.syncValue(arrayValue)
        }
    }]

    syncValue(arrayValue){
        let outputNodes = this.outputs
        if (outputNodes != null && outputNodes.length > 0) {
            for (let output of outputNodes) {
                if (!arrayValue.includes(output.name)) {
                    let slotId = this.findOutputSlot(output.name);
                    this.removeOutput(slotId);
                }
            }
        }

        for (let val of arrayValue) {
            let slotId = this.findOutputSlot(val);
            if (slotId == -1) {
                this.addOutput(val, "string");
                slotId = this.findOutputSlot(val); // Find again to retrieve the correct slotId.
            }

            this.setOutputData(slotId, val)
        }

        this.graph.afterChange(this);
    }

    onConfigure(o: SerializedLGraphNode) {
        let contents = o.properties.contents
        if(contents != null && contents.length > 0){
            this.syncValue(contents)
        }
    }

    static getType(): string {
        return "basic/constStringArray"
    }
}

LiteGraph.registerNodeType(ConstStringArrayNode.getType(), ConstStringArrayNode)
export {ConstStringArrayNode}