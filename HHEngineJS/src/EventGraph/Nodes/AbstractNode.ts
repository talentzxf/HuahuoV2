import {LGraphNode} from "litegraph.js";
import {NodeTargetType} from "../GraphActions";

class AbstractNode extends LGraphNode {
    eventGraphComponent

    setEventGraphComponent(eventGraphComponent) {
        this.eventGraphComponent = eventGraphComponent
    }

    getEventGraphComponent() {
        return this.eventGraphComponent
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