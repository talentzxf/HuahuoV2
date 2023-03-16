import {HHForm} from "../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {LGraph, LGraphCanvas, LiteGraph} from "litegraph.js";

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
        form.style.width = CANVAS_WIDTH*1.2 + "px"

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
        resetScaleButton.onclick = function(e){
            _this.lcanvas.ds.changeScale(1.0);
            _this.graph.change()
            e.preventDefault()
        }

        form.appendChild(resetScaleButton)
    }

    initLGraph(canvas: HTMLCanvasElement) {
        this.graph = new LGraph()
        this.lcanvas = new LGraphCanvas(canvas, this.graph, {autoresize: false})

        var node_const = LiteGraph.createNode("basic/const");
        node_const.pos = [200, 200];
        this.graph.add(node_const);
        node_const.setValue(4.5);

        var node_watch = LiteGraph.createNode("basic/watch");
        node_watch.pos = [700, 200];
        this.graph.add(node_watch);

        node_const.connect(0, node_watch, 0);

        this.graph.start()
    }

}

export {EventGraphForm}