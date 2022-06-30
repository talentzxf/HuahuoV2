import {CustomElement, Logger} from "hhcommoncomponents"
import {renderEngine2D, huahuoEngine} from "hhenginejs"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {HHTimeline} from "hhtimeline"
import {ResizeObserver} from 'resize-observer';
import {defaultShapeDrawer} from "../ShapeDrawers/Shapes";
import {EditorPlayer} from "./EditorPlayer";
import {fileLoader} from "./FileLoader";

@CustomElement({
    selector: "hh-sceneview"
})
class SceneView extends HTMLElement {
    canvasContainer: HTMLDivElement
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D = null;

    currentShapeDrawer: BaseShapeDrawer = defaultShapeDrawer;

    gizmoContainer: HTMLDivElement = null;
    zoomInBtn: HTMLButtonElement = null;
    zoomOutBtn: HTMLButtonElement = null;
    editorPlayer: EditorPlayer = null;
    storeId: number = null;

    createCanvasContainer() {
        this.canvasContainer = document.createElement("div")
        // this.canvasContainer.style.width = "100%"
        // this.canvasContainer.style.height = "100%"
        this.canvasContainer.style.padding = "0"
        this.canvasContainer.style.margin = "0"
        this.canvasContainer.style.backgroundColor = "gray"
        this.canvasContainer.style.flexBasis = "100%"
        this.canvasContainer.style.overflowY = "auto"

        this.appendChild(this.canvasContainer)
    }

    createCanvas() {
        this.canvas = document.createElement("canvas")
        this.canvas.setAttribute("resize", "true")
        this.canvas.style.padding = "0"
        this.canvas.style.margin = "0"
        this.canvasContainer.appendChild(this.canvas)

        this.ctx = this.canvas.getContext("2d")

        this.canvas.addEventListener("dragover", (e)=>{
            e.stopPropagation()
            e.preventDefault()
            e.dataTransfer.dropEffect = "copy"
        })

        this.canvas.addEventListener("drop", (e)=>{
            e.stopPropagation()
            e.preventDefault()
            const fileList = e.dataTransfer.files;
            fileLoader.loadImageFile(fileList[0])
        })
    }

    setupEventsAndCreateFirstTrack() {
        window.addEventListener("resize", this.OnResize.bind(this))

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.canvasContainer)

        EventBus.getInstance().on(EventNames.DRAWSHAPEBEGINS, this.beginToDrawShape.bind(this))
        EventBus.getInstance().on(EventNames.DRAWSHAPEENDS, this.endOfDrawingShape.bind(this))

        this.canvas.addEventListener("mousedown", this.onMouseDown.bind(this))
        this.canvas.addEventListener("mousemove", this.onMouseMove.bind(this))
        this.canvas.addEventListener("mouseup", this.onMouseUp.bind(this))

        let timeline: HHTimeline = document.querySelector("hh-timeline")
        timeline.addNewTrack()
    }

    createGizmos() {
        this.gizmoContainer = document.createElement("div")
        this.gizmoContainer.style.position = "absolute"
        this.gizmoContainer.style.display = "flex"
        this.gizmoContainer.style.flexDirection = "column"
        this.appendChild(this.gizmoContainer)

        this.zoomInBtn = document.createElement("button")
        this.zoomInBtn.innerHTML = "ZoomIn"
        this.zoomInBtn.onclick = () => {
            renderEngine2D.zoomIn()
        }
        this.gizmoContainer.appendChild(this.zoomInBtn)

        this.zoomOutBtn = document.createElement("button")
        this.zoomOutBtn.innerHTML = "ZoomOut"
        this.zoomOutBtn.onclick = () => {
            renderEngine2D.zoomOut()
        }
        this.gizmoContainer.appendChild(this.zoomOutBtn)

        this.zoomOutBtn = document.createElement("button")
        this.zoomOutBtn.innerHTML = "Reset"
        this.zoomOutBtn.onclick = () => {
            renderEngine2D.zoomReset()
        }
        this.gizmoContainer.appendChild(this.zoomOutBtn)
    }

    connectedCallback() {
        this.style.width = "100%"
        this.style.height = "100%"

        this.createCanvasContainer()
        this.createCanvas()
        renderEngine2D.init(this.canvas)

        this.createGizmos();
        this.setupEventsAndCreateFirstTrack()

        this.editorPlayer = new EditorPlayer()

        this.storeId = huahuoEngine.GetCurrentStore().GetStoreId()

        defaultShapeDrawer.onBeginToDrawShape(this.canvas)
    }

    onMouseDown(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.editorPlayer.isPlaying) {
            this.currentShapeDrawer.onMouseDown(evt)
        }
    }

    onMouseMove(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.editorPlayer.isPlaying) {
            this.currentShapeDrawer.onMouseMove(evt)
        }
    }

    onMouseUp(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.editorPlayer.isPlaying) {
            this.currentShapeDrawer.onMouseUp(evt)
        }
    }

    beginToDrawShape(shapeDrawer: BaseShapeDrawer) {
        this.currentShapeDrawer = shapeDrawer;
        this.currentShapeDrawer.onBeginToDrawShape(this.canvas)
    }

    endOfDrawingShape(shapeDrawer: BaseShapeDrawer) {
        this.currentShapeDrawer = defaultShapeDrawer;
        this.canvas.style.cursor = "default"
    }

    OnResize() {
        let containerWidth = this.canvasContainer.clientWidth
        let containerHeight = this.canvasContainer.clientHeight
        Logger.debug("OnResize: ContainerWidth:" + containerWidth + ", ContainerHeight:" + containerHeight)

        let margin = 10
        let canvasWidth = containerWidth - margin
        let canvasHeight = containerHeight - margin

        // if(canvasHeight > containerHeight){
        //     canvasHeight = containerHeight * margin
        //     canvasWidth = canvasHeight * this.aspectRatio
        // }

        this.canvas.width = canvasWidth
        this.canvas.height = canvasHeight
        this.canvas.style.width = canvasWidth + "px"
        this.canvas.style.height = canvasHeight + "px"
        this.canvas.style.position = "relative"
        this.canvas.style.left = (containerWidth - canvasWidth) / 2 + "px"
        this.canvas.style.top = (containerHeight - canvasHeight) / 2 + "px"
        this.Redraw()
    }

    Redraw() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height)
        this.ctx.fillStyle = "white"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        renderEngine2D.clearBackground()
    }

}

export {SceneView}