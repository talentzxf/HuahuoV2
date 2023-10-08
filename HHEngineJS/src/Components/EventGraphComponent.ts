import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {LGraph} from "litegraph.js";
import {huahuoEngine} from "../EngineAPI";
import {capitalizeFirstLetter, eventBus, IsValidWrappedObject} from "hhcommoncomponents";
import {EventNode} from "../EventGraph/Nodes/EventNode";
import {ActionNode} from "../EventGraph/Nodes/ActionNode";
import {setupLGraph} from "../EventGraph/LGraphSetup";

declare var Module: any;

@Component({compatibleShapes: ["BaseShapeJS"], cppClassName: "EventGraphComponent"})
class EventGraphComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", null, true)
    eventGraphJSON

    // This corresponds to the button editing the event graph.
    @PropertyValue(PropertyCategory.customField)
    eventGraph

    graph: LGraph

    // If shape is null, this node is listening to global event.
    linkNodeWithTarget(nodeId: number, actionTarget: BaseShapeJS | AbstractComponent) {
        if (actionTarget != null)
            this.rawObj.AddNodeIdObjectMap(nodeId, actionTarget.getRawObject())
        else
            this.rawObj.AddNodeIdObjectMap(nodeId, null)
    }

    getBaseActor() {
        return this.baseShape.getActor()
    }

    getActionTarget(nodeId: number) {
        let rawObj = this.rawObj.GetObjectByNodeId(nodeId)

        let rawObjType = rawObj.GetType()
        let baseShapeType = rawObjType.FindTypeByName("BaseShape")
        let customComponentType = rawObjType.FindTypeByName("CustomComponent")

        // The target object might be inside another element, need to perform recusvice search
        if (rawObjType.IsDerivedFrom(baseShapeType)) {
            let baseShapeObj = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(rawObj, true)

            this.baseShape.getActor().AddActionInvoker(this)

            return baseShapeObj.getActor()
        } else if (rawObjType.IsDerivedFrom(customComponentType)) {
            let customComponentRawObj = Module.wrapPointer(rawObj.ptr, Module.CustomComponent)
            let baseShapeRawObj = customComponentRawObj.GetBaseShape()

            let baseShapeObj = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(baseShapeRawObj, true)

            let componentObj = baseShapeObj.getComponentByRawObj(customComponentRawObj)

            return componentObj
        }
    }

    getEventBusForNode(nodeId: number) {
        let rawObj = this.rawObj.GetObjectByNodeId(nodeId)
        if (rawObj == null || !IsValidWrappedObject(rawObj))
            return eventBus

        // This is a component.
        if (rawObj.GetType().GetName() == "CustomComponent") {
            let componentRawObj = Module.wrapPointer(rawObj.ptr, Module.CustomComponent)
            let baseShapeRawObj = componentRawObj.GetBaseShape()

            // The target object might be inside another element, need to perform recusvice search
            let baseShapeObj = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(baseShapeRawObj, true)
            let componentObj = baseShapeObj.getComponentByRawObj(componentRawObj)
            return huahuoEngine.getEvent(componentObj).getEventBus()
        }

        let baseShapeObj = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(rawObj, true)
        return huahuoEngine.getEvent(baseShapeObj).getEventBus()
    }

    saveGraph() {
        let graphString = JSON.stringify(this.graph.serialize())
        if (this.eventGraphJSON != graphString)
            this.eventGraphJSON = graphString
    }

    getInputValueFunction(propertyName) {
        if (!this.hasOwnProperty(propertyName))
            return null
        return this[propertyName]
    }

    setInputValueFunction(propertyName, propertyValue) {
        if (!this.hasOwnProperty(propertyName))
            return

        let setterName = "set" + capitalizeFirstLetter(propertyName)

        if (this[setterName])
            this[setterName](propertyValue)
    }

    constructor(rawObj?, isMirage: boolean = false) {
        let needLoad = rawObj ? true : false;
        super(rawObj, isMirage);

        this.graph = new LGraph()
        if (this.eventGraphJSON && this.eventGraphJSON.length > 0) {
            let data = JSON.parse(this.eventGraphJSON)
            this.graph.configure(data)
        }

        // this.graph.start()
        this.graph["onAfterChange"] = this.saveGraph.bind(this)

        this.graph["getInputValueFunction"] = this.getInputValueFunction.bind(this)
        this.graph["setInputValueFunction"] = this.setInputValueFunction.bind(this)

        if (needLoad) {
            let eventNodes = this.graph.findNodesByType(EventNode.getType())
            for (let node of eventNodes) {
                let eventNode = node as EventNode
                eventNode.setEventGraphComponent(this)
            }

            let actionNodes = this.graph.findNodesByType(ActionNode.getType())
            for (let actionNode of actionNodes) {
                (actionNode as ActionNode).setEventGraphComponent(this)
            }
        }
    }

    getGraph() {
        return this.graph
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (huahuoEngine.getActivePlayer().isPlaying) {
            this.graph.start()
        } else {
            this.graph.stop()
        }
    }

    reset() {
        super.reset();

        let eventNodes = this.graph.findNodesByType(EventNode.getType())
        for (let node of eventNodes) {
            let eventNode = node as EventNode
            eventNode.reset()
        }

    }
}

setupLGraph()

export {EventGraphComponent}