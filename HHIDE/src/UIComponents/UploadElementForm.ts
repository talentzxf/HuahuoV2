import {HHForm} from "../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {renderEngine2D, huahuoEngine, Player} from "hhenginejs"
import {RenderPreviewCanvas} from "./ProjectInfoForm";

// TODO: A lot of duplication code with ProjectInfoForm.
@CustomElement({
    selector: "hh-upload-element-list"
})
class UploadElementForm extends HTMLElement implements HHForm {
    selector: string;

    onOKAction: (isShareable: boolean, isEditable: boolean) => void;

    listDiv
    closeBtn: HTMLButtonElement
    previewCanvas: HTMLCanvasElement
    previewCanvasContainer: HTMLDivElement
    previewAnimationPlayer: Player
    elementNameSpan: HTMLSpanElement

    okBtn: HTMLButtonElement
    cancelBtn: HTMLButtonElement

    eleStoreId: string
    eleName: string

    setStore(storeId, name) {
        this.eleStoreId = storeId
        this.eleName = name

        if (this.elementNameSpan) {
            this.elementNameSpan.innerText = this.eleName
        }

        this.RedrawFrame()
    }

    connectedCallback() {
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"

        this.innerHTML += "<style>" +
            "ul{" +
            "list-style-type: none;" +
            "width: 500px" +
            "}" +
            "li img {" +
            "  float: left;" +
            "  margin: 0 15px 0 0;" +
            "}" +
            "</style>"

        this.listDiv = document.createElement("div")
        this.listDiv.innerHTML = CSSUtils.formStyle
        this.listDiv.innerHTML +=
            "   <form id='projectListForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='closeBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "   <h4>Upload Element <span id='elementName'>" + this.eleName + "</span></h4>" +
            "   <div id='preview-canvas-container'>" +
            "       <canvas id='preview-canvas' style='border: 1px solid blue'></canvas>" +
            "   </div>" +
            "   <h4>Do you want to share the element?</h4>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='share' name='shareable' value='true' checked>" +
            "   <label for='share'>Share</label>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='unshare' name='shareable' value='false'>" +
            "   <label for='share'>Unshare</label>" +
            "   <h4>Do you want others to edit this element?</h4>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='editable' name='editable' value='true' checked>" +
            "   <label for='share'>Editable</label>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='uneditable' name='editable' value='false'>" +
            "   <label for='share'>UnEditable</label>" +
            "   <input style='background-color: #6396D8' id='Submit' type='button' value='Submit'>" +
            "   <input style='background-color: #6396D8' id='Cancel' type='button' value='Cancel'>" +
            "   </form>"
        this.appendChild(this.listDiv)

        this.elementNameSpan = this.listDiv.querySelector("#elementName")

        this.previewCanvasContainer = this.listDiv.querySelector("#preview-canvas-container")
        this.previewCanvas = this.listDiv.querySelector("#preview-canvas")
        this.closeBtn = this.listDiv.querySelector("#closeBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.previewAnimationPlayer = new Player()

        let prevCanvas = renderEngine2D.getDefaultCanvas()
        renderEngine2D.init(this.previewCanvas, true)
        if (prevCanvas)
            renderEngine2D.setDefaultCanvas(prevCanvas)

        let [initW, initH] = renderEngine2D.getInitCanvasWH()

        if (initW > 0) {
            renderEngine2D.resize(this.previewCanvas, initW, initH)
        }

        window.addEventListener("resize", this.OnResize.bind(this))

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.previewCanvasContainer)

        this.okBtn = this.querySelector("#Submit")
        this.cancelBtn = this.querySelector("#Cancel")

        this.okBtn.onclick = this.OnOK.bind(this)
        this.cancelBtn.onclick = this.closeForm.bind(this)

        this.okBtn.onmouseenter = this.mouseEnter.bind(this)
        this.okBtn.onmouseout = this.mouseOutBtn.bind(this)
        this.cancelBtn.onmouseenter = this.mouseEnter.bind(this)
        this.cancelBtn.onmouseout = this.mouseOutBtn.bind(this)
    }

    mouseEnter(evt: MouseEvent) {
        (evt.target as HTMLElement).style.backgroundColor = '#267ded'
    }

    mouseOutBtn(evt: MouseEvent) {
        (evt.target as HTMLElement).style.backgroundColor = '#6396D8'
    }

    OnOK(evt) {
        evt.stopPropagation()
        evt.preventDefault()

        if (this.onOKAction) {
            let isShareable: boolean = this.listDiv.querySelector('input[name="shareable"]:checked').value == "true"
            let isEditable: boolean = this.listDiv.querySelector('input[name="editable"]:checked').value == "true"
            this.onOKAction(isShareable, isEditable)
        }

        this.closeForm()
    }

    // Duplicate with ProjectInfo.
    OnResize() {
        if (window.getComputedStyle(this.parentElement).display == "none")
            return;

        let containerWidth = this.previewCanvasContainer.clientWidth
        let containerHeight = this.previewCanvasContainer.clientHeight

        if (containerWidth <= 0 || containerHeight <= 0)
            return

        let margin = 0
        let proposedCanvasWidth = containerWidth - margin
        let proposedCanvasHeight = containerHeight - margin

        let [actualCanvasWidth, actualCanvasHeight] = renderEngine2D.getContentWH(proposedCanvasWidth, proposedCanvasHeight)

        renderEngine2D.resize(this.previewCanvas, actualCanvasWidth, actualCanvasHeight)

        this.previewCanvas.width = actualCanvasWidth
        this.previewCanvas.height = actualCanvasHeight
        this.previewCanvas.style.width = actualCanvasWidth + "px"
        this.previewCanvas.style.height = actualCanvasHeight + "px"
        this.previewCanvas.style.position = "relative"
        this.previewCanvas.style.left = (containerWidth - actualCanvasWidth) / 2 + "px"
        this.previewCanvas.style.top = (containerHeight - actualCanvasHeight) / 2 + "px"
        this.RedrawFrame()
    }

    RedrawFrame() {
        if (null == this.eleStoreId)
            return

        let currentLayer = huahuoEngine.GetCurrentLayer()
        let currentFrameId = currentLayer.GetCurrentFrame()

        RenderPreviewCanvas(this.eleStoreId, this.previewAnimationPlayer, this.previewCanvas, currentFrameId)
    }

    closeForm() {
        this.style.display = "none"
    }

}

export {UploadElementForm}