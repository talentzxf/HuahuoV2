import {HHForm} from "../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {LGraph, LGraphCanvas, LiteGraph} from "litegraph.js";
import {eventBus} from "hhcommoncomponents";
import {getLiteGraphTypeFromPropertyType} from "./Utils"
import {EventNode} from "./Nodes/EventNode";

let CANVAS_WIDTH = 800
let CANVAS_HEIGHT = 600

@CustomElement({
    selector: "hh-event-graph-form"
})
class EventGraphForm extends HTMLElement implements HHForm {
    selector: string;
    containerDiv: HTMLDivElement
    closeBtn: HTMLDivElement

    graph: LGraph
    lcanvas: LGraphCanvas

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

        let canvas = document.createElement("canvas")
        canvas.width = CANVAS_WIDTH
        canvas.height = CANVAS_HEIGHT
        canvas.style.width = CANVAS_WIDTH + "px"
        canvas.style.height = CANVAS_HEIGHT + "px"
        form.appendChild(canvas)

        let resetScaleButton = document.createElement("button")
        resetScaleButton.innerText = i18n.t("eventgraph.resetScale")
        resetScaleButton.style.width = "100px"
        this.appendChild(this.containerDiv)

        this.initLGraph(canvas)

        let _this = this
        resetScaleButton.onclick = function (e) {
            _this.lcanvas.ds.changeScale(1.0);
            _this.graph.change()
            e.preventDefault()
        }

        form.appendChild(resetScaleButton)
    }

    eventListenerMenu(node, options, e, prev_menu, callback) {
        if (!this.lcanvas)
            return

        if (!this.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()
        let events = eventBus.getAllGlobalEvents()

        let entries = []

        let _this = this
        events.forEach((stringValue) => {
            let entry = {
                value: "events/eventNode",
                content: stringValue,
                has_submenu: false,
                callback: function(value, event, mouseEvent, contextMenu){
                    let first_event = contextMenu.getFirstEvent();
                    let graph = _this.graph
                    let lcanvas = _this.lcanvas
                    graph.beforeChange()

                    let node = LiteGraph.createNode(value.value) as EventNode
                    if(node){
                        let paramDefs = eventBus.getEventParameters(stringValue) || []
                        for(let paramDef of paramDefs){
                            let outputSlot = node.addOutput(paramDef.parameterName, getLiteGraphTypeFromPropertyType(paramDef.parameterType))
                            node.addParameterIndexSlotMap(paramDef.paramIndex, outputSlot)
                        }

                        node.setFullEventName(stringValue)
                        node.pos = lcanvas.convertEventToCanvasOffset(first_event)
                        lcanvas.graph.add(node)
                    }

                    if(callback)
                        callback(node)

                    graph.afterChange()
                }
            }

            entries.push(entry)
        })

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }

    initLGraph(canvas: HTMLCanvasElement) {
        this.graph = new LGraph()
        this.lcanvas = new LGraphCanvas(canvas, this.graph, {autoresize: false})

        let _this = this
        this.lcanvas.getExtraMenuOptions = function () {
            let options = [{
                content: i18n.t("eventgraph.addEventListener"),
                has_submenu: true,
                callback: _this.eventListenerMenu.bind(_this)
            }]

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

        this.graph.start()
    }

}

export {EventGraphForm}