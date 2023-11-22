import {AbstractNode} from "../Nodes/AbstractNode";
import {LiteGraph} from "litegraph.js";

class CompareNode extends AbstractNode {
    title = "Compare Node"
    desc = "evaluates condition between A and B"

    compareValues(): boolean {
        let A = this.getInputData(1, true);
        let B = this.getInputData(2, true);

        let result = false;
        if (typeof A == typeof B) {
            switch (this.properties.OP) {
                case "==":
                case "!=":
                    // traverse both objects.. consider that this is not a true deep check! consider underscore or other library for thath :: _isEqual()
                    result = true;
                    switch (typeof A) {
                        case "object":
                            var aProps = Object.getOwnPropertyNames(A);
                            var bProps = Object.getOwnPropertyNames(B);
                            if (aProps.length != bProps.length) {
                                result = false;
                                break;
                            }
                            for (var i = 0; i < aProps.length; i++) {
                                var propName = aProps[i];
                                if (A[propName] !== B[propName]) {
                                    result = false;
                                    break;
                                }
                            }
                            break;
                        default:
                            result = A == B;
                    }
                    if (this.properties.OP == "!=") result = !result;
                    break;
            }
        }

        return result
    }

    static values = ["==", "!="]; //[">", "<", "==", "!=", "<=", ">=", "||", "&&" ];

    getTitle(): string {
        return "*A " + this.properties.OP + " *B"
    }

    constructor() {
        super();

        this.addInput("Execute", LiteGraph.EVENT)
        // @ts-ignore
        this.addInput("A", 0)
        // @ts-ignore
        this.addInput("B", 0)

        this.addOutput("true", LiteGraph.EVENT)
        this.addOutput("false", LiteGraph.EVENT)

        this.addProperty("OP", "==", "enum", {values: CompareNode.values});
    }

    onAction(action, param) {
        if (this.compareValues()) {
            this.triggerSlot(0, null, null)
        } else {
            this.triggerSlot(1, null, null)
        }
    }

    static getType(): string {
        return "logic/compare"
    }
}

LiteGraph.registerNodeType(CompareNode.getType(), CompareNode)
export {CompareNode}