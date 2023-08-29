import {BaseForm} from "./BaseForm";
import {CustomElement} from "hhcommoncomponents";
import {Player, renderEngine2D, huahuoEngine} from "hhenginejs"
import {CSSUtils} from "../Utilities/CSSUtils";
import {SceneView} from "../SceneView/SceneView";
import {RenderPreviewCanvas} from "./ProjectInfoForm";
import {GlobalConfig} from "hhenginejs";
import {saveAs} from 'file-saver';

function getPaperJs(){
    // if(paper.project)
    //     return paper
    return window.paper
}

@CustomElement({
    selector: "hh-export-image"
})
class ExportImageForm extends BaseForm {
    previewCanvas: HTMLCanvasElement
    previewCanvasContainer: HTMLDivElement

    previewAnimationPlayer: Player

    listDiv: HTMLDivElement
    exportBtn: HTMLButtonElement
    closeBtn: HTMLButtonElement

    scaleInput: HTMLInputElement
    nameInput: HTMLInputElement

    form: HTMLFormElement

    originalFormWidth: number = 0
    originalFormHeight: number = 0
    originalCanvasContainerWidth: number = 0
    originalCanvasContainerHeight: number = 0

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
            "   <form id='exportImageForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='closeBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "   <div class='input-group mb-3'>" +
            "       <div class='input-group-prepend'>" +
            "           <span class='input-group-text' id='basic-addon1'>Scale</span>" +
            "       </div>" +
            "       <input id='scale' type='number' style='margin: 0px; height:38px' class='form-control' min='0.1' max='2.0' step='0.1' value='1.0' aria-label='scale' aria-describedby='basic-addon1'>" +
            "   </div>" +
            "   <div class='input-group mb-3'>" +
            "       <div class='input-group-prepend'>" +
            "           <span class='input-group-text' id='basic-addon2'>Name</span>" +
            "       </div>" +
            "       <input id='name' type='text' style='margin: 0px; height:38px' class='form-control' value='huahuo_exported.gif' aria-label='name' aria-describedby='basic-addon2'>" +
            "   </div>" +
            "   <div id='preview-canvas-container'>" +
            "       <canvas id='preview-canvas' style='border: 1px solid blue'></canvas>" +
            "   </div>" +
            "   <input style='background-color: #6396D8' id='Export' type='button' value='Export'>" +
            "   <input style='background-color: #6396D8' id='Cancel' type='button' value='Cancel'>" +
            "   </form>"
        this.appendChild(this.listDiv)

        this.form = this.listDiv.querySelector("form")

        this.previewCanvas = this.listDiv.querySelector("#preview-canvas")
        this.previewCanvasContainer = this.listDiv.querySelector("#preview-canvas-container")
        this.exportBtn = this.listDiv.querySelector("#Export")
        this.exportBtn.addEventListener("mousedown", this.exportImage.bind(this))

        this.nameInput = this.listDiv.querySelector("#name")
        this.scaleInput = this.listDiv.querySelector("#scale")
        this.scaleInput.addEventListener("change", this.onScaleChanged.bind(this))
        this.closeBtn = this.listDiv.querySelector("#closeBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.originalFormWidth = this.form.clientWidth
        this.originalFormHeight = this.form.clientHeight
        this.originalCanvasContainerWidth = this.previewCanvasContainer.clientWidth
        this.originalCanvasContainerHeight = this.previewCanvasContainer.clientHeight

        this.previewAnimationPlayer = new Player()
        this.previewAnimationPlayer.isInEditor = false

        let prevCanvas = renderEngine2D.getDefaultCanvas()
        renderEngine2D.init(this.previewCanvas, true)
        if (prevCanvas) {
            renderEngine2D.setDefaultCanvas(prevCanvas)
        }

        let [initW, initH] = renderEngine2D.getInitCanvasWH()

        if (initW > 0) {
            renderEngine2D.resize(this.previewCanvas, initW, initH)
        }

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.previewCanvasContainer)
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

    waterMarkText = null
    drawWaterMark(){
        let prevCanvas = renderEngine2D.setDefaultCanvas(this.previewCanvas)

        try{
            if(this.waterMarkText == null){
                let paper = getPaperJs()
                this.waterMarkText = new paper.PointText(new paper.Point(0,100))
                this.waterMarkText.fontSize = 50
                this.waterMarkText.fillColor = new paper.Color("black")
                this.waterMarkText.strokeColor = new paper.Color("black")
                this.waterMarkText.content = "Created By: https://www.huahuo.online"
            }

            let globalDim = renderEngine2D.getGlobalPosition(this.previewCanvas.width, this.previewCanvas.height)

            this.waterMarkText.position.x = globalDim.x - this.waterMarkText.getBounds().width
            this.waterMarkText.position.y = globalDim.y - this.waterMarkText.getBounds().height
        }finally {
            renderEngine2D.setDefaultCanvas(prevCanvas)
        }

    }

    RedrawFrame(frameId = null) {
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let mainStoreId = mainSceneView.storeId
        let currentLayer = huahuoEngine.GetCurrentLayer()
        if (frameId == null) {
            frameId = currentLayer.GetCurrentFrame()
        }

        RenderPreviewCanvas(mainStoreId, this.previewAnimationPlayer, this.previewCanvas, frameId)

        this.drawWaterMark()
    }

    onScaleChanged(){
        let newScale = Number.parseFloat(this.scaleInput.value)

        if(newScale >= 1.0){
            this.form.style.width = this.originalFormWidth * newScale + "px"
            this.previewCanvasContainer.style.width = this.originalCanvasContainerWidth * newScale + "px"
        }else{
            this.previewCanvasContainer.style.width = this.originalCanvasContainerWidth * newScale + "px"
        }
    }

    exportImage() {
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let mainStoreId = mainSceneView.storeId

        let prevPlayer = huahuoEngine.getActivePlayer()
        let currentStoreId = huahuoEngine.GetCurrentStoreId()
        let currentFrameId = huahuoEngine.GetCurrentLayer().GetCurrentFrame()

        let previousCanvas = null

        let maxFrames = huahuoEngine.getStoreMaxFrames(mainStoreId)

        previousCanvas = renderEngine2D.setDefaultCanvas(this.previewCanvas)
        // @ts-ignore
        let gif = new GIF({
            workers: 1,
            quality: 10,
            workerScript: "static/gif.js/dist/gif.worker.js"
        })

        gif.on("finished", (blob) => {
            console.log("Get the file!")

            let name = _this.nameInput.value
            if(name == null){
                name = "huahuo_exported.gif"
            }

            if(!name.endsWith(".gif")){
                name = name + ".gif"
            }

            saveAs(blob, name)
        })

        let frameId = 0
        let _this = this

        let resolveHandler: Function = null
        let captureGifFramePromise = new Promise((resolve, reject) => {
            resolveHandler = resolve
        })

        let captureGifFrameStep = function () {
            _this.previewAnimationPlayer.setFrameId(frameId)
            _this.drawWaterMark()
            gif.addFrame(_this.previewCanvas, {delay: 1000.0 / GlobalConfig.fps, copy: true})

            if (frameId < maxFrames) {
                requestAnimationFrame(captureGifFrameStep)
            } else {
                resolveHandler()
            }
            frameId++
        }

        captureGifFramePromise.then(() => {
            gif.render()

            if(previousCanvas)
                renderEngine2D.setDefaultCanvas(previousCanvas)
            huahuoEngine.setActivePlayer(prevPlayer)
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(currentStoreId)
            huahuoEngine.getActivePlayer().setFrameId(currentFrameId)
        })
        requestAnimationFrame(captureGifFrameStep)
    }
}

export {ExportImageForm}