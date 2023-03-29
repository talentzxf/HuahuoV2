import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseShapeEvents} from "../EventGraph/BaseShapeEvents";
import {LGraph} from "litegraph.js";
import {EventEmitter} from "hhcommoncomponents";
import {EventNode} from "../EventGraph/Nodes/EventNode";

@Component({cppClassName: "EventGraphComponent"})
class EventGraphComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", null, true)
    eventGraphJSON

    // This corresponds to the button editing the event graph.
    @PropertyValue(PropertyCategory.customField)
    eventGraph

    @PropertyValue(PropertyCategory.shapeArray, null, null, true)
    listenedObjects

    graph: LGraph

    // TODO: Need persist of following arrays.
    actions: Map<BaseShapeJS, BaseShapeActions> = new Map

    eventEmitters: Map<BaseShapeJS, BaseShapeEvents> = new Map

    saveGraph() {
        let graphString = JSON.stringify(this.graph.serialize())
        if (this.eventGraphJSON != graphString)
            this.eventGraphJSON = graphString

        let eventNodes = this.graph.findNodesByType(EventNode.name)
        for (let node of eventNodes) {
            let eventNode = node as EventNode
            let eventNodeId = eventNode.id
            let targetEventBus = eventNode.getTargetEventBus()

            this.rawObj
        }
    }

    constructor(rawObj?) {
        let needLoad = rawObj ? true : false;
        super(rawObj);

        this.graph = new LGraph()
        if (this.eventGraphJSON && this.eventGraphJSON.length > 0) {
            let data = JSON.parse(this.eventGraphJSON)
            this.graph.configure(data)
        }

        this.graph.start()
        this.graph["onAfterChange"] = this.saveGraph.bind(this)

        if (needLoad) {
            // Init the listened objects.
            for (let shape of this.listenedObjects) {
                this.getAction(shape)
            }
        }
    }

    getGraph() {
        return this.graph
    }

    getAction(baseShape: BaseShapeJS) {
        if (!this.actions.has(baseShape)) {
            this.actions.set(baseShape, new BaseShapeActions(baseShape))
        }
        return this.actions.get(baseShape)
    }

    getEvent(baseShape: BaseShapeJS): EventEmitter {
        if (!this["containsListenedObjects"](baseShape))
            this["insertListenedObjects"](baseShape)

        if (!this.eventEmitters.has(baseShape)) {
            this.eventEmitters.set(baseShape, new BaseShapeEvents(baseShape))
        }
        return this.eventEmitters.get(baseShape)
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        this.actions.forEach(action => {
            action.execute()
        })
    }
}

export {EventGraphComponent}