import {BaseForm} from "./BaseForm";
import {CustomElement} from "hhcommoncomponents";
import {Player, renderEngine2D, huahuoEngine} from "hhenginejs"
import {CSSUtils} from "../Utilities/CSSUtils";
import {SceneView} from "../SceneView/SceneView";
import {RenderPreviewCanvas} from "./ProjectInfoForm";
import {GlobalConfig} from "hhenginejs";
import {saveAs} from 'file-saver';

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
            "   <div id='preview-canvas-container'>" +
            "       <canvas id='preview-canvas' style='border: 1px solid blue'></canvas>" +
            "   </div>" +
            "   <input style='background-color: #6396D8' id='Export' type='button' value='Export'>" +
            "   <input style='background-color: #6396D8' id='Cancel' type='button' value='Cancel'>" +
            "   </form>"
        this.appendChild(this.listDiv)

        this.previewCanvas = this.listDiv.querySelector("#preview-canvas")
        this.previewCanvasContainer = this.listDiv.querySelector("#preview-canvas-container")
        this.exportBtn = this.listDiv.querySelector("#Export")
        this.exportBtn.addEventListener("mousedown", this.exportImage.bind(this))
        this.closeBtn = this.listDiv.querySelector("#closeBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.previewAnimationPlayer = new Player()
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

    RedrawFrame(frameId = null) {
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let mainStoreId = mainSceneView.storeId
        let currentLayer = huahuoEngine.GetCurrentLayer()
        if (frameId == null) {
            frameId = currentLayer.GetCurrentFrame()
        }

        RenderPreviewCanvas(mainStoreId, this.previewAnimationPlayer, this.previewCanvas, frameId)
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

            saveAs(blob, "huahuo_exported.gif")
        })

        let frameId = 0
        let _this = this

        let resolveHandler: Function = null
        let captureGifFramePromise = new Promise((resolve, reject) => {
            resolveHandler = resolve
        })

        let captureGifFrameStep = function () {
            _this.previewAnimationPlayer.setFrameId(frameId)

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

            huahuoEngine.setActivePlayer(prevPlayer)
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(currentStoreId)
            huahuoEngine.getActivePlayer().setFrameId(currentFrameId)

            renderEngine2D.setDefaultCanvas(previousCanvas)
        })
        requestAnimationFrame(captureGifFrameStep)
    }
}

export {ExportImageForm}