import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {LGraph} from "litegraph.js";
import {huahuoEngine} from "../EngineAPI";
import {capitalizeFirstLetter, eventBus, IsValidWrappedObject} from "hhcommoncomponents";
import {EventNode} from "../EventGraph/Nodes/EventNode";
import {ActionNode} from "../EventGraph/Nodes/ActionNode";
import {setupLGraph} from "../EventGraph/LGraphSetup";
import {NodeTargetType} from "../EventGraph/GraphActions";
import {PlayerActions} from "../Player/PlayerActions";

declare var Module: any;

@Component({compatibleShapes: ["BaseShapeJS"], cppClassName: "EventGraphComponent"})
class EventGraphComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", null, true)
    eventGraphJSON

    // This corresponds to the button editing the event graph.
    @PropertyValue(PropertyCategory.customField)
    eventGraph

    needReloadGraph = true
    graph: LGraph = new LGraph()

    // If shape is null, this node is listening to global event.
    linkNodeWithTarget(nodeId: number, type: NodeTargetType, actionTarget: BaseShapeJS | AbstractComponent) {
        if (actionTarget != null)
            this.rawObj.AddNodeIdObjectMap(nodeId, type, actionTarget.getRawObject())
        else
            this.rawObj.AddNodeIdObjectMap(nodeId, type, null)
    }

    getBaseActor() {
        return this.baseShape.getActor()
    }

    playerAction = new PlayerActions(this)

    getActionTarget(nodeId: number) {
        let rawObj = this.rawObj.GetObjectByNodeId(nodeId)

        let objType = this.rawObj.GetNodeIdObjectType(nodeId)
        if (objType == NodeTargetType.PLAYER) {
            return this.playerAction
        }

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
        if (this.eventGraphJSON != graphString){
            this.eventGraphJSON = graphString

            this.needReloadGraph = true // After save, need reload the graph.
        }
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

    reloadGraph() {
        if (this.needReloadGraph) {
            if (this.eventGraphJSON && this.eventGraphJSON.length > 0) {
                let data = JSON.parse(this.eventGraphJSON)
                this.graph.configure(data)
            }

            // this.graph.start()
            this.graph["onAfterChange"] = this.saveGraph.bind(this)

            this.graph["getInputValueFunction"] = this.getInputValueFunction.bind(this)
            this.graph["setInputValueFunction"] = this.setInputValueFunction.bind(this)

            let eventNodes = this.graph.findNodesByType(EventNode.getType())
            for (let node of eventNodes) {
                let eventNode = node as EventNode
                eventNode.setEventGraphComponent(this)
            }

            let actionNodes = this.graph.findNodesByType(ActionNode.getType())
            for (let actionNode of actionNodes) {
                (actionNode as ActionNode).setEventGraphComponent(this)
            }

            this.needReloadGraph = false
        }
    }

    getGraph() {
        return this.graph
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        this.reloadGraph()

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

        this.needReloadGraph = true
    }
}

setupLGraph()

export {EventGraphComponent}