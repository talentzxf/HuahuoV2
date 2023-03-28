import {HHForm} from "../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {eventBus} from "hhcommoncomponents";
import {getLiteGraphTypeFromPropertyType} from "./Utils"
import {EventNode, ActionNode} from "hhenginejs";
import {BaseShapeActions} from "hhenginejs";
import {LGraphCanvas, LiteGraph} from "hhenginejs";

let CANVAS_WIDTH = 800
let CANVAS_HEIGHT = 600

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

    baseShapeActions: BaseShapeActions

    setTargetComponent(targetComponent) {
        this.targetComponent = targetComponent

        // Get Actions from the baseShape.
        this.baseShapeActions = this.targetComponent.getAction(this.targetComponent.baseShape)

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

        this.containerDiv.innerHTML += "<form id='eventGraphContainer'> " +
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

    actionListenerMenu(node, options, e, prev_menu, callback){
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        if(!this.baseShapeActions)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        let _this = this
        let actionDefs = this.baseShapeActions.getActionDefs()
        let entries = []
        actionDefs.forEach((actionDef) => {
            let entry = {
                value: "actions/actionNode",
                content: actionDef.actionName,
                has_submenu: false,
                callback: function (value, event, mouseEvent, contextMenu) {
                    let first_event = contextMenu.getFirstEvent();
                    let graph = _this.lcanvas.graph
                    let lcanvas = _this.lcanvas
                    graph.beforeChange()

                    let node = LiteGraph.createNode(value.value) as ActionNode
                    if (node) {
                        let paramDefs = actionDef.paramDefs
                        for (let paramDef of paramDefs) {
                            let outputSlot = node.addInput(paramDef.paramName, getLiteGraphTypeFromPropertyType(paramDef.paramType))
                            node.addParameterIndexSlotMap(paramDef.paramIndex, outputSlot)
                        }

                        node.setActionName(actionDef.actionName)
                        node.pos = lcanvas.convertEventToCanvasOffset(first_event)
                        lcanvas.graph.add(node)

                        node.setActionTarget(_this.baseShapeActions)
                    }

                    if (callback)
                        callback(node)

                    graph.afterChange()
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

        // Build up events
        let events = eventBus.getAllGlobalEvents()

        let localEvents = this.targetComponent.getEvent(this.targetComponent.baseShape).getEvents()

        for(let eventName of localEvents)
            events.push(eventName)

        let entries = []

        let _this = this
        events.forEach((stringValue) => {
            let entry = {
                value: "events/eventNode",
                content: stringValue,
                has_submenu: false,
                callback: function (value, event, mouseEvent, contextMenu) {
                    let first_event = contextMenu.getFirstEvent();
                    let graph = _this.lcanvas.graph
                    let lcanvas = _this.lcanvas
                    graph.beforeChange()

                    let node = LiteGraph.createNode(value.value) as EventNode
                    if (node) {
                        let paramDefs = eventBus.getEventParameters(stringValue) || []
                        for (let paramDef of paramDefs) {
                            let outputSlot = node.addOutput(paramDef.parameterName, getLiteGraphTypeFromPropertyType(paramDef.parameterType))
                            node.addParameterIndexSlotMap(paramDef.paramIndex, outputSlot)
                        }

                        node.setFullEventName(stringValue)
                        node.pos = lcanvas.convertEventToCanvasOffset(first_event)
                        lcanvas.graph.add(node)
                    }

                    if (callback)
                        callback(node)

                    graph.afterChange()
                }
            }

            entries.push(entry)
        })

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }

    initLGraph(canvas: HTMLCanvasElement) {
        let graph = this.targetComponent.getGraph()

        if(this.lcanvas == null){
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
                        callback: _this.actionListenerMenu.bind(_this)
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
        }
        else
            this.lcanvas.setGraph(graph)
    }

}

export {EventGraphForm}