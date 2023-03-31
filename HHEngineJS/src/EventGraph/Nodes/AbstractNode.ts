import {LGraphNode} from "litegraph.js";

class AbstractNode extends LGraphNode{
    eventGraphComponent
    setEventGraphComponent(eventGraphComponent){
        this.eventGraphComponent = eventGraphComponent
    }

    getEventGraphComponent(){
        return this.eventGraphComponent
    }

    static getType(): string{
        return "unknowNode"
    }
}

export {AbstractNode}