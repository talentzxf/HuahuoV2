import {HHForm} from "../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {renderEngine2D, huahuoEngine, Player} from "hhenginejs"
import {SceneView} from "../SceneView/SceneView";

@CustomElement({
    selector: "hh-upload-element-list"
})
class UploadElementForm extends HTMLElement implements HHForm{
    selector: string;

    afterOKAction: Function;

    listDiv
    closeBtn: HTMLButtonElement
    previewCanvas: HTMLCanvasElement
    previewCanvasContainer: HTMLDivElement
    previewAnimationPlayer: Player

    connectedCallback(){
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
            "   <h3>Upload Element</h3>" +
            "   <div id='preview-canvas-container'>" +
            "       <canvas id='preview-canvas' style='border: 1px solid blue'></canvas>" +
            "   </div>" +
            "   <h4>Do you want to share the element?</h4>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='share' name='shareable' value='share' checked>" +
            "   <label for='share'>Share</label>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='unshare' name='shareable' value='unshare'>" +
            "   <label for='share'>Unshare</label>" +
            "   <h4>Do you want others to edit this element?</h4>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='editable' name='editable' value='editable' checked>" +
            "   <label for='share'>Editable</label>" +
            "   <input style='display: inline; width: auto; height: auto' type='radio' id='uneditable' name='editable' value='uneditable'>" +
            "   <label for='share'>UnEditable</label>" +
            "   <input style='background-color: #6396D8' id='Submit' type='button' value='Submit'>" +
            "   <input style='background-color: #6396D8' id='Cancel' type='button' value='Cancel'>" +
            "   </form>"
        this.appendChild(this.listDiv)

        this.previewCanvasContainer = this.listDiv.querySelector("#preview-canvas-container")
        this.previewCanvas = this.listDiv.querySelector("#preview-canvas")
        this.closeBtn = this.listDiv.querySelector("#closeBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.previewAnimationPlayer = new Player()
        
        let prevCanvas = renderEngine2D.getDefaultCanvas()
        renderEngine2D.init(this.previewCanvas)
        if(prevCanvas)
            renderEngine2D.setDefaultCanvas(prevCanvas)

        let [initW, initH] = renderEngine2D.getInitCanvasWH()

        if(initW > 0){
            renderEngine2D.resize(this.previewCanvas, initW, initH)
        }

        window.addEventListener("resize", this.OnResize.bind(this))

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.previewCanvasContainer)

    }

    // Duplicate with ProjectInfo.
    OnResize(){
        if(window.getComputedStyle(this.parentElement).display == "none")
            return;

        let containerWidth = this.previewCanvasContainer.clientWidth
        let containerHeight = this.previewCanvasContainer.clientHeight

        if(containerWidth <= 0 || containerHeight <= 0)
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

    RedrawFrame(){
        let prevStore = huahuoEngine.GetCurrentStoreId()

        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let mainStoreId = mainSceneView.storeId
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainStoreId) // Only render the main store.i.e. the 1st store.

        let currentLayer = huahuoEngine.GetCurrentLayer()
        let currentFrameId = currentLayer.GetCurrentFrame()
        let previousCanvas = renderEngine2D.setDefaultCanvas(this.previewCanvas)

        this.previewAnimationPlayer.storeId = mainStoreId
        this.previewAnimationPlayer.loadShapesFromStore()
        this.previewAnimationPlayer.setFrameId(currentFrameId)
        if(previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)

        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStore)
    }

    closeForm() {
        this.style.display = "none"
    }

}

export {UploadElementForm}