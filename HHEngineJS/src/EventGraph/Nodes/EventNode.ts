import {LiteGraph, SerializedLGraphNode} from "litegraph.js";
import {i18n, splitFullEventName} from "hhcommoncomponents";
import {AbstractNode} from "./AbstractNode";
import {huahuoEngine} from "../../EngineAPI";
import {NodeTargetType} from "../GraphActions";
import {eventBus} from "hhcommoncomponents";

class EventNode extends AbstractNode {
    title = "EventNode"
    desc = "Triggers if event happens"

    properties = {
        fullEventName: null,
        paramIdxOutputSlotMap: {}
    }

    currentEventHandler = -1

    constructor() {
        super();
        this.addOutput("Execute", LiteGraph.EVENT, {
            label: i18n.t("eventgraph.Execute")
        })
    }

    eventHasBeenSet: boolean = false

    unsubscribeEvent() {
        let fullEventName = this.properties.fullEventName
        let eventNameMeta = splitFullEventName(fullEventName)

        let targetEventBus = this.getTargetEventBus()
        if (this.properties.fullEventName && this.currentEventHandler > 0) {
            targetEventBus.removeEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, this.currentEventHandler)
            this.currentEventHandler = -1
        }
    }

    reset() {
        this.eventHasBeenSet = false
        this.unsubscribeEvent()
    }

    onExecute() {
        if (!this.eventHasBeenSet) {
            this.setupEvent()

            this.eventHasBeenSet = true
        }
    }

    addParameterIndexSlotMap(paramIdx, outputSlot) {
        this.getParamMap()[paramIdx] = outputSlot
    }

    getParamMap() {
        return this.properties.paramIdxOutputSlotMap
    }

    setFullEventName(fullEventName: string, title = null) {
        this.properties.fullEventName = fullEventName
        this.title = title || fullEventName
    }

    getTargetEventBus() {
        let targetType = this.properties["targetTypeInfo"].type
        if (targetType == null) {
            console.error("Node is not inited???")
            return
        }

        let additionalInfo = this.properties["targetTypeInfo"].additionalInfo

        let baseShape = this.getBaseShape()
        switch (targetType) {
            case NodeTargetType.SHAPE:
                return huahuoEngine.getEvent(baseShape).getEventBus()
            case NodeTargetType.PLAYER:
            case NodeTargetType.CANVAS:
                return eventBus;
            case NodeTargetType.COMPONENT:
                let componentIdx = additionalInfo["componentId"]
                let componentRawObj = baseShape.getRawObject().GetFrameStateByIdx(componentIdx)
                let component = baseShape.getComponentByRawObj(componentRawObj)
                return huahuoEngine.getEvent(component).getEventBus()
            case NodeTargetType.GRAPHCOMPONENT:
                return huahuoEngine.getEvent(this.getEventGraphComponent()).getEventBus()
            default:
                console.warn("Target type is not defined!!!")
        }
    }

    setupEvent() {
        this.unsubscribeEvent()

        let fullEventName = this.properties.fullEventName
        let eventNameMeta = splitFullEventName(fullEventName)

        let targetEventBus = this.getTargetEventBus()

        let _this = this
        this.currentEventHandler = targetEventBus.addEventHandler(eventNameMeta.namespace, eventNameMeta.eventName, (params) => {
            if (!huahuoEngine.getActivePlayer().isPlaying) // Do not trigger when player is not playing.
                return

            for (let paramIdx = 0; paramIdx < params.length; paramIdx++) {
                let slot = _this.getParamMap()[paramIdx]
                if (slot) {
                    let slotIdx = _this.findOutputSlot(slot.name)
                    if (slotIdx >= 0) {
                        _this.setOutputData(slotIdx, params[paramIdx]);
                    }
                }
            }
            _this.triggerSlot(0, null, null)
            _this.setDirtyCanvas(true, true)
        })
    }

    onRemoved() {
        this.unsubscribeEvent()
    }

    // This function will be called after s/l from file.
    onConfigure(o: SerializedLGraphNode) {
        console.log("on Node configured")

        // this.setupEvent(this.properties.fullEventName)
    }

    static getType(): string {
        return "events/eventNode"
    }
}

LiteGraph.registerNodeType(EventNode.getType(), EventNode)

export {EventNode}