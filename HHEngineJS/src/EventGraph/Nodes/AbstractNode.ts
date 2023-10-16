import {LGraphNode} from "litegraph.js";
import {NodeTargetType} from "../GraphActions";
import {huahuoEngine} from "../../EngineAPI";

class AbstractNode extends LGraphNode {
    getEventGraphComponent() {
        return this.graph["component"]
    }

    getBaseShape() {
        return this.graph["component"].baseShape
    }

    setEventTargetType(type: NodeTargetType, additionalInfo) {
        this.properties["targetTypeInfo"] = {
            type: type,
            additionalInfo: additionalInfo
        }
    }

    static getType(): string {
        return "unknowNode"
    }
}

export {AbstractNode}