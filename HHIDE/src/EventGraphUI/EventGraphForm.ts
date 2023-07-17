import {HHForm} from "../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {getFullEventName} from "hhcommoncomponents";
import {getEventCategoryMap, getLiteGraphTypeFromPropertyType} from "./Utils"
import {EventNode, ActionNode} from "hhenginejs";
import {LGraphCanvas, LiteGraph} from "hhenginejs";
import {huahuoEngine} from "hhenginejs";
import {renderEngine2D} from "hhenginejs"
import {ActionDef} from "hhenginejs";

let CANVAS_WIDTH = 800
let CANVAS_HEIGHT = 600

LiteGraph.slot_types_default_out["shape"] = ["shape/getComponentByTypeName"]

@CustomElement({
    selector: "hh-event-graph-form"
})
class EventGraphForm extends HTMLElement implements HHForm {
    canvas: HTMLCanvasElement

    selector: string;
    containerDiv: HTMLDivElement
    closeBtn: HTMLDivElement

    lcanvas: LGraphCanvas

    targetComponent

    setTargetComponent(targetComponent) {
        this.targetComponent = targetComponent

        this.initLGraph(this.canvas)
    }

    closeForm() {
        this.style.display = "none"
    }

    connectedCallback() {
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"
        this.containerDiv = document.createElement("div")
        this.containerDiv.innerHTML = CSSUtils.formStyle

        this.containerDiv.innerHTML += "<style>" +
            ".litegraph .dialog.settings {" +
            "height: calc( 50% - 20px );" +
            "max-width: 50%;" +
            "top: 50%" +
            "}" +
            "</style>"

        this.containerDiv.innerHTML += "<form id='eventGraphContainer' class='litegraph litegraph-editor'> " +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='closeBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "</form>"

        let form = this.containerDiv.querySelector("form")
        form.style.width = CANVAS_WIDTH * 1.2 + "px"

        this.closeBtn = this.containerDiv.querySelector("#closeBtn")
        this.closeBtn.onclick = this.closeForm.bind(this)

        this.canvas = document.createElement("canvas")
        this.canvas.width = CANVAS_WIDTH
        this.canvas.height = CANVAS_HEIGHT
        this.canvas.style.width = CANVAS_WIDTH + "px"
        this.canvas.style.height = CANVAS_HEIGHT + "px"
        form.appendChild(this.canvas)

        let resetScaleButton = document.createElement("button")
        resetScaleButton.innerText = i18n.t("eventgraph.resetScale")
        resetScaleButton.style.width = "100px"
        this.appendChild(this.containerDiv)

        let _this = this
        resetScaleButton.onclick = function (e) {
            _this.lcanvas.ds.changeScale(1.0);
            _this.lcanvas.graph.change()
            e.preventDefault()
        }

        form.appendChild(resetScaleButton)

        this.addEventListener("mousedown", this.onMouseDown.bind(this))
        this.addEventListener("mouseup", this.onMouseUp.bind(this))
    }

    isDragging: boolean = false
    mouseX: number = -1
    mouseY: number = -1

    onMouseDown(e: MouseEvent) {
        this.isDragging = true

        document.addEventListener("mousemove", this.onDrag.bind(this))

        this.mouseX = e.clientX
        this.mouseY = e.clientY
    }

    onDrag(e: MouseEvent) {
        if (this.isDragging && e.buttons == 1) { // Is dragging and left mouse is true.
            let offsetX = e.clientX - this.mouseX
            let offsetY = e.clientY - this.mouseY

            this.style.left = this.offsetLeft + offsetX + "px"
            this.style.top = this.offsetTop + offsetY + "px"
            this.mouseX = e.clientX
            this.mouseY = e.clientY
        }
    }

    onMouseUp() {
        this.isDragging = false
        this.mouseX = -1
        this.mouseY = -1

        document.removeEventListener("mousemove", this.onDrag.bind(this))
    }

    actionCallBack(value, event, mouseEvent, contextMenu, callback, actionDef:ActionDef, actionTarget) {
        let first_event = contextMenu.getFirstEvent();
        let graph = this.lcanvas.graph
        let lcanvas = this.lcanvas
        graph.beforeChange()

        let node = LiteGraph.createNode(value.value) as ActionNode
        if (node) {
            let paramDefs = actionDef.paramDefs
            for (let paramDef of paramDefs) {
                let inputSlot = node.addInput(paramDef.paramName, getLiteGraphTypeFromPropertyType(paramDef.paramType))
                node.addParameterIndexSlotMap(paramDef.paramIndex, inputSlot)
            }

            if(actionDef.returnValueInfo != null){
                let returnValueName = actionDef.returnValueInfo.valueName
                let returnValueType = actionDef.returnValueInfo.valueType

                node.addOutput(returnValueName, getLiteGraphTypeFromPropertyType(returnValueType))
            }

            node.pos = lcanvas.convertEventToCanvasOffset(first_event)
            lcanvas.graph.add(node)

            // TODO: Whatif we need to perform action on another object?
            this.targetComponent.linkNodeWithTarget(node.id, actionTarget)
            node.setEventGraphComponent(this.targetComponent)
            node.setActionDef(actionDef)
        }

        if (callback)
            callback(node)

        graph.afterChange()
    }

    componentActionMenu(node, options, e, prev_menu, callback){
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        let baseShape = this.targetComponent.baseShape
        let _this = this
        let entries = []

        let components = baseShape.getComponents()

        if(components.length > 0){
            for (let component of components) {
                component.getActionDefs().forEach((actionDef) => {
                    let entry = {
                        value: "actions/actionNode",
                        content: component.getTypeName() + "/" + actionDef.actionName,
                        has_submenu: false,
                        callback: function (value, event, mouseEvent, contextMenu) {
                            _this.actionCallBack(value, event, mouseEvent, contextMenu, callback, actionDef, component)
                        }
                    }

                    entries.push(entry)
                })
            }


            new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
        }
    }

    actionMenu(node, options, e, prev_menu, callback) {
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        let baseShape = this.targetComponent.baseShape
        let _this = this
        let actionDefs = baseShape.getAction().getActionDefs()

        let entries = []
        actionDefs.forEach((actionDef) => {
            let entry = {
                value: "actions/actionNode",
                content: actionDef.actionName,
                has_submenu: false,
                callback: function (value, event, mouseEvent, contextMenu) {
                    _this.actionCallBack(value, event, mouseEvent, contextMenu, callback, actionDef, baseShape)
                }
            }

            entries.push(entry)
        })

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }


    eventListenerMenu(node, options, e, prev_menu, callback) {
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        // Map from the event name to the object that triggers the event.
        // If the value is null, the event is triggered in global eventBus.
        let eventNameEventBusMap = new Map

        let player = huahuoEngine.getActivePlayer()
        // Build up player events
        let events = player.getEventBus().getAllEvents()
        for (let eventName of events) {
            eventNameEventBusMap.set(eventName, null)
        }

        // Build up renderEngine events
        let renderEngineEvents = renderEngine2D.getEventBus().getAllEvents()
        for (let eventName of renderEngineEvents) {
            eventNameEventBusMap.set(eventName, null)
        }

        let localEvents = huahuoEngine.getEvent(this.targetComponent.baseShape).getEvents()

        for (let eventName of localEvents) {
            eventNameEventBusMap.set(eventName, this.targetComponent.baseShape)
        }

        let namespaceCategories = getEventCategoryMap(eventNameEventBusMap)

        let entries = []

        let _this = this
        namespaceCategories.forEach((eventObjects, namespace) => {
            let entry = {
                value: namespace,
                content: namespace,
                has_submenu: true,
                callback: function (value, event, mouseEvent, contextMenu) {
                    let eventEntries = []

                    eventObjects.forEach((eventObject: object) => {
                        let eventEntry = {
                            value: "events/eventNode",
                            content: eventObject["eventName"],
                            has_submenu: false,
                            callback: function (value, event, mouseEvent, contextMenu) {
                                let fullEventName = getFullEventName(namespace, eventObject["eventName"])

                                let first_event = contextMenu.getFirstEvent();
                                let graph = _this.lcanvas.graph
                                let lcanvas = _this.lcanvas
                                graph.beforeChange()

                                let node = LiteGraph.createNode(value.value) as EventNode
                                if (node) {
                                    let paramDefs = huahuoEngine.getEventBus(eventObject["eventSource"]).getEventParameters(fullEventName) || []
                                    for (let paramDef of paramDefs) {
                                        let outputSlot = node.addOutput(paramDef.parameterName, getLiteGraphTypeFromPropertyType(paramDef.parameterType))
                                        node.addParameterIndexSlotMap(paramDef.paramIndex, outputSlot)
                                    }

                                    node.pos = lcanvas.convertEventToCanvasOffset(first_event)
                                    lcanvas.graph.add(node)

                                    _this.targetComponent.linkNodeWithTarget(node.id, eventObject["eventSource"])
                                    node.setEventGraphComponent(_this.targetComponent)

                                    node.setupEvent(fullEventName)
                                }

                                if (callback)
                                    callback(node)

                                graph.afterChange()
                            }
                        }
                        eventEntries.push(eventEntry)
                    })

                    new LiteGraph.ContextMenu(eventEntries, {event: e, parentMenu: contextMenu}, ref_window)
                }
            }

            entries.push(entry)
        })

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }

    initLGraph(canvas: HTMLCanvasElement) {
        let graph = this.targetComponent.getGraph()

        if (this.lcanvas == null) {
            this.lcanvas = new LGraphCanvas(canvas, graph, {autoresize: false})
            let _this = this
            this.lcanvas.getExtraMenuOptions = function () {
                let options = [
                    {
                        content: i18n.t("eventgraph.addEventListener"),
                        has_submenu: true,
                        callback: _this.eventListenerMenu.bind(_this)
                    },
                    {
                        content: i18n.t("eventgraph.addGraphAction"),
                        has_submenu: true,
                        callback: _this.actionMenu.bind(_this)
                    },
                    {
                        content: i18n.t("eventgraph.addComponentGraphAction"),
                        has_submenu: true,
                        callback: _this.componentActionMenu.bind(_this)
                    }
                ]

                return options
            }

            // var node_const = LiteGraph.createNode("basic/const");
            // node_const.pos = [200, 200];
            // this.graph.add(node_const);
            // node_const.setValue(4.5);
            //
            // var node_watch = LiteGraph.createNode("basic/watch");
            // node_watch.pos = [700, 200];
            // this.graph.add(node_watch);
            //
            // node_const.connect(0, node_watch, 0);

            LiteGraph["release_link_on_empty_shows_menu"] = true
        } else
            this.lcanvas.setGraph(graph)
    }

}

export {EventGraphForm}