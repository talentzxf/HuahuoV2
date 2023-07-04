import {CustomElement, Logger} from "hhcommoncomponents"
import {renderEngine2D, huahuoEngine} from "hhenginejs"
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {HHTimeline} from "hhtimeline"
import {ResizeObserver} from 'resize-observer';
import {defaultShapeDrawer} from "../ShapeDrawers/Shapes";
import {EditorPlayer} from "../AnimationPlayer/EditorPlayer";
import {fileLoader} from "./FileLoader";
import {findParentContent, findParentPanel, HHSideBar} from "hhpanel";
import {sceneViewManager} from "./SceneViewManager";
import {SVGFiles} from "../Utilities/Svgs";

function allReadyExecute(fn: Function) {
    i18n.ExecuteAfterInited(
        () => {
            huahuoEngine.ExecuteAfterInited(
                fn
            )
        }
    )
}

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
    zoomResetBtn: HTMLButtonElement = null;
    animationPlayer: EditorPlayer = null;
    storeId: string;

    inited: boolean = false;
    timeline: HHTimeline = null;

    setStoreId(storeId: string) {
        this.storeId = storeId
        this.animationPlayer.storeId = storeId
    }

    createCanvasContainer() {
        this.canvasContainer = document.createElement("div")
        this.canvasContainer.id = "CanvasContainer"
        // this.canvasContainer.style.width = "100%"
        // this.canvasContainer.style.height = "100%"
        this.canvasContainer.style.padding = "0"
        this.canvasContainer.style.margin = "0"
        this.canvasContainer.style.backgroundColor = "lightgray"
        this.canvasContainer.style.flexBasis = "100%"
        this.canvasContainer.style.overflowY = "auto"

        this.appendChild(this.canvasContainer)
    }

    createCanvas() {
        this.canvas = document.createElement("canvas")
        this.canvas.setAttribute("resize", "true")

        this.canvas.className = "SceneViewCanvas"
        this.canvas.setAttribute("resize", "true")
        this.canvas.style.padding = "0"
        this.canvas.style.margin = "0"
        this.canvas.classList.add("dockable")
        this.canvasContainer.appendChild(this.canvas)

        this.ctx = this.canvas.getContext("2d")

        this.canvas.addEventListener("dragover", (e) => {
            e.stopPropagation()
            e.preventDefault()
            e.dataTransfer.dropEffect = "copy"
        })

        this.canvas.addEventListener("drop", (e) => {
            e.stopPropagation()
            e.preventDefault()
            const fileList = e.dataTransfer.files;
            fileLoader.loadImageFile(fileList[0])
        })
    }

    isPanning = false

    OnKeyDown(e: KeyboardEvent) {
        if (e.key == "Escape") {
            this.endOfDrawingShape(this.currentShapeDrawer)
        } else {
            if (e.ctrlKey) {
                this.canvas.style.cursor = "grabbing"
                this.isPanning = true
            }
        }
    }

    OnKeyUp(e: KeyboardEvent) {
        if (!e.ctrlKey) {
            this.canvas.style.cursor = "default"
            this.isPanning = false
        }
    }

    setupEventsAndCreateFirstTrack() {
        window.addEventListener("resize", this.OnResize.bind(this))
        window.addEventListener("keydown", this.OnKeyDown.bind(this))
        window.addEventListener("keyup", this.OnKeyUp.bind(this))

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.canvasContainer)

        IDEEventBus.getInstance().on(EventNames.DRAWSHAPEBEGINS, this.beginToDrawShape.bind(this))
        IDEEventBus.getInstance().on(EventNames.DRAWSHAPEENDS, this.endOfDrawingShape.bind(this))

        this.canvas.addEventListener("mousedown", this.onMouseDown.bind(this))
        this.canvas.addEventListener("mousemove", this.onMouseMove.bind(this))
        this.canvas.addEventListener("mouseup", this.onMouseUp.bind(this))
        this.canvas.addEventListener("wheel", this.onWheel.bind(this))
        this.canvas.addEventListener("dblclick", this.onDbClick.bind(this))


        if (this.timeline == null)
            this.timeline = document.createElement("hh-timeline") as HHTimeline

        this.canvasContainer.insertBefore(this.timeline, this.canvas)

        this.timeline.contextMenu.setItems([
            {
                itemName: i18n.t("contextmenu.mergecells"),
                onclick: this.timeline.mergeCells.bind(this.timeline)
            },
            {
                itemName: i18n.t("contextmenu.createNewTrack"),
                onclick: function (e) {
                    this.createNewTrack()
                }
            },
            {
                itemName: i18n.t("contextmenu.markAsAnimationEnd"),
                onclick: function (e) {
                    this.markAsAnimationEnd(this.timeline)
                }
            }
        ])

        let currentStore = huahuoEngine.GetCurrentStore()
        // If no layer in the store now, create a new track.
        let layerCount = currentStore.GetLayerCount()
        if (layerCount == 0)
            this.createNewTrack()
        else {
            // Set up icons. In some cases, layers are created else where (like in elementCreator) and icons are not setup during layer creation
            for (let layerId = 0; layerId < layerCount; layerId++) {
                let layer = currentStore.GetLayer(layerId)
                let eyeIcon = this.createEyeIcon()
                this.timeline.setLayerIcons(layer, [eyeIcon])
            }
            this.timeline.reloadTracks()
        }


        if (this.animationPlayer == null) {
            this.animationPlayer = new EditorPlayer(this)
            huahuoEngine.setActivePlayer(this.animationPlayer)
        }
    }

    onWheel(evt: WheelEvent) {
        if (evt.ctrlKey) {
            evt.stopPropagation()
            evt.preventDefault()

            let mousePoint = new paper.Point(evt.offsetX, evt.offsetY)

            let curView = (window.paper as any).view
            let oldZoom = curView.zoom
            let oldCenter = curView.center
            evt.deltaY > 0 ? renderEngine2D.zoomOut(1.01) : renderEngine2D.zoomIn(1.01)
            let newZoom = curView.zoom

            let newViewCenterDelta = (mousePoint.subtract(oldCenter)).multiply(1.0 - oldZoom / newZoom)
            curView.center = curView.center.add(newViewCenterDelta)
        }
    }

    markAsAnimationEnd(timeline: HHTimeline) {
        let titleTrack = timeline.getTrack(0)
        if (titleTrack == null) {
            Logger.error("No title bar??")
            return;
        }
        let cellId = titleTrack.getCurrentCellId()
        console.log("Set max animation frame as:" + cellId)

        let firstTrack = timeline.getTrack(1)
        if (firstTrack == null) {
            Logger.error("No track in the timeline??")
            return;
        }

        firstTrack.getLayer().GetObjectStore().UpdateMaxFrameId(cellId, true)
    }

    createEyeIcon() {
        let _this = this
        let eyeIcon = new Image()
        eyeIcon.src = SVGFiles.eyeSvg
        eyeIcon["onIconClicked"] = function (layer) {
            let currentlyVisible = layer.GetIsVisible()
            if (currentlyVisible) { // Currently visible
                eyeIcon.src = SVGFiles.eyeSlashSvg
            } else {
                eyeIcon.src = SVGFiles.eyeSvg
            }

            layer.SetIsVisible(!currentlyVisible)
        }

        eyeIcon.onload = function () {
            _this.timeline.reloadTracks()
        }

        return eyeIcon
    }

    createNewTrack() {
        let eyeIcon = this.createEyeIcon()
        this.timeline.reloadTracks()
        let track = this.timeline.addNewTrack(null, [eyeIcon])
        this.timeline.selectTrack(track.getSeqId(), null)
    }

    createGizmos() {
        this.gizmoContainer = document.createElement("div")
        this.gizmoContainer.style.position = "absolute"
        this.gizmoContainer.style.display = "flex"
        this.gizmoContainer.style.flexDirection = "column"
        this.appendChild(this.gizmoContainer)

        this.zoomInBtn = document.createElement("button")
        this.zoomInBtn.innerHTML = (window as any).i18n.t("zoomin")
        this.zoomInBtn.style.userSelect = 'none'
        this.zoomInBtn.onclick = () => {
            renderEngine2D.zoomIn()
        }
        this.gizmoContainer.appendChild(this.zoomInBtn)

        this.zoomOutBtn = document.createElement("button")
        this.zoomOutBtn.innerHTML = (window as any).i18n.t("zoomout")
        this.zoomOutBtn.style.userSelect = 'none'
        this.zoomOutBtn.onclick = () => {
            renderEngine2D.zoomOut()
        }
        this.gizmoContainer.appendChild(this.zoomOutBtn)

        this.zoomResetBtn = document.createElement("button")
        this.zoomResetBtn.innerHTML = (window as any).i18n.t("reset")
        this.zoomResetBtn.style.userSelect = 'none'
        this.zoomResetBtn.onclick = () => {
            renderEngine2D.zoomReset()
        }
        this.gizmoContainer.appendChild(this.zoomResetBtn)
    }

    // drawCoordinate(){
    //     let yCoord = new paper.Path.Line(new paper.Point(0.0, 0.0), new paper.Point(0.0, 1000.0))
    //     yCoord.strokeColor = new paper.Color("Black")
    //     yCoord.strokeWidth = 10
    //     let xCoord = new paper.Path.Line(new paper.Point(0.0, 0.0), new paper.Point(1000.0, 0.0))
    //     xCoord.strokeColor = new paper.Color("Black")
    //     xCoord.strokeWidth = 10
    // }

    initSceneView() {
        if (!this.inited) {
            this.style.width = "100%"
            this.style.height = "100%"

            this.createCanvasContainer()
            this.createCanvas()
            renderEngine2D.init(this.canvas);

            this.createGizmos()

            this.setupEventsAndCreateFirstTrack()

            defaultShapeDrawer.onBeginToDrawShape(this.canvas)

            this.storeId = huahuoEngine.GetCurrentStore().GetStoreId()
            console.log("New store id:" + this.storeId)

            if (!this.animationPlayer) {
                this.animationPlayer = new EditorPlayer(this)
                huahuoEngine.setActivePlayer(this.animationPlayer)
            }
            this.animationPlayer.storeId = this.storeId

            this.inited = true

            sceneViewManager.registerSceneView(this)

            sceneViewManager.focusSceneView(this)

            // Refresh sidebar content.
            let sidebars = document.querySelectorAll("hh-sidebar")
            for (let sidebar of sidebars) {
                (sidebar as HHSideBar).refreshDockables()
            }
        }
    }

    connectedCallback() {
        allReadyExecute(this.initSceneView.bind(this))
    }

    onDbClick(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.isPlaying) {
            this.currentShapeDrawer.onDblClick(evt)
        }
    }

    resetDefaultShapeDrawer() {
        defaultShapeDrawer.onBeginToDrawShape(this.canvas)
    }

    get isPlaying(): boolean {
        return null != this.animationPlayer && this.animationPlayer.isPlaying
    }

    panningStartPoint: paper.Point
    onMouseDown(evt: MouseEvent) {
        // Operating in this sceneview, set the storeId of this sceneview as default.
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId)

        // Inform the panel to select my tab
        // Find the panel
        let panel = findParentPanel(this)
        // Find the content
        let content = findParentContent(this)
        let title = content.getTitle()
        panel.selectTab(title.tabIndex)

        if (evt.ctrlKey) {
            this.isPanning = true
            this.panningStartPoint = new paper.Point(evt.offsetX, evt.offsetY)
        } else if (this.currentShapeDrawer && !this.isPlaying) {
            this.currentShapeDrawer.onMouseDown(evt)
        }
    }

    onMouseMove(evt: MouseEvent) {
        if (!this.isPanning) {
            if (this.currentShapeDrawer && !this.isPlaying) {
                this.currentShapeDrawer.onMouseMove(evt)
            }
        } else {
            if(this.panningStartPoint != null){
                let curView = (window.paper as any).view

                let currentPoint = curView.viewToProject(new paper.Point(evt.offsetX, evt.offsetY))
                let panningStart = curView.viewToProject(this.panningStartPoint)
                let delta = panningStart.subtract(currentPoint)
                curView.center = delta.add(curView.center)

                this.panningStartPoint = new paper.Point(evt.offsetX, evt.offsetY)
            }
        }
    }

    onMouseUp(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.isPlaying) {
            this.currentShapeDrawer.onMouseUp(evt)
            this.panningStartPoint = null
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
        if (window.getComputedStyle(this.parentElement).display == "none")
            return;

        // Find the panel
        let panel = findParentPanel(this.canvasContainer)

        if (panel == null)  // The scene view might has already been closed
            return;

        // TODO: Move this into HHPanel??
        let panelWidth = panel.clientWidth
        let panelHeight = panel.clientHeight
        let titleHeight = panel.querySelector(".title_tabs").clientHeight

        let timelineHeight = this.timeline ? this.timeline.canvasScrollContainer.offsetHeight : 0;
        let contentHeight = panelHeight - titleHeight - timelineHeight;

//        let containerWidth = this.canvasContainer.clientWidth
//        let containerHeight = this.canvasContainer.clientHeight
//        Logger.debug("OnResize: ContainerWidth:" + containerWidth + ", ContainerHeight:" + containerHeight)

        let containerWidth = this.canvasContainer.clientWidth
        let containerHeight = contentHeight
        let margin = 15
        let canvasWidth = containerWidth - margin
        let canvasHeight = containerHeight - margin

        renderEngine2D.resize(this.canvas, canvasWidth, canvasHeight)

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

        if (this.gizmoContainer)
            this.gizmoContainer.style.top = this.canvas.offsetHeight + timelineHeight + "px"

        // Refresh sidebar content.
        let sidebars = document.querySelectorAll("hh-sidebar")
        for (let sidebar of sidebars) {
            (sidebar as HHSideBar).refreshDockables()
        }
    }

    Redraw() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height)
        this.ctx.fillStyle = "white"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        let previousCanvas = renderEngine2D.setDefaultCanvas(this.canvas)
        renderEngine2D.clearBackground()
        if (previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)
    }

}

export {SceneView}