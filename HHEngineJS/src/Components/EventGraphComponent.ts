import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
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
    reloading = false

    graph: LGraph = new LGraph()

    getBaseActor() {
        return this.baseShape.getActor()
    }

    playerAction = new PlayerActions(this)

    constructor(rawObj?) {
        super(rawObj);

        this.graph["component"] = this
    }

    saveGraph() {
        if (this.reloading) // During reloading, no need to trigger save.
            return

        let graphString = JSON.stringify(this.graph.serialize())
        if (this.eventGraphJSON != graphString) {
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

    addInput(inputName: string, inputType: string) {
        if (this.hasOwnProperty(inputName)) {
            console.log("Input " + inputName + " already existed!")
            return false;
        }
        console.log("Input added")
        // TODO: Switch - case?? Looks stupid, need to seed some more elegant way to do this.
        switch (inputType) {
            case "number":
                let propertyDef: PropertyDef = {
                    key: inputName,
                    type: PropertyCategory.interpolateFloat,
                    initValue: 0.0,
                    hide: false,
                    config: null
                }
                this.addProperty(propertyDef, true)
                break;
            default:
                console.log("Unknown property:" + inputType)
                break;
        }

        return true
    }

    reloadGraph() {
        if (this.needReloadGraph) {
            if (this.eventGraphJSON && this.eventGraphJSON.length > 0) {
                this.reloading = true
                let data = JSON.parse(this.eventGraphJSON)
                this.graph.configure(data)
                this.reloading = false

                let totalInputCount = this.graph.inputs.length
                for (let inputIdx = 0; totalInputCount < totalInputCount; totalInputCount++) {
                    let inputValue = this.graph.inputs.getInput(inputIdx)
                    this.addInput(inputValue.name, inputValue.type)
                }
            }

            // this.graph.start()
            this.graph["onAfterChange"] = this.saveGraph.bind(this)

            this.graph["getInputValueFunction"] = this.getInputValueFunction.bind(this)
            this.graph["setInputValueFunction"] = this.setInputValueFunction.bind(this)

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