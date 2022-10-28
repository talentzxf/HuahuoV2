import {CustomElement, Logger} from "hhcommoncomponents"
import {renderEngine2D, huahuoEngine} from "hhenginejs"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {HHTimeline} from "hhtimeline"
import {ResizeObserver} from 'resize-observer';
import {defaultShapeDrawer} from "../ShapeDrawers/Shapes";
import {EditorPlayer} from "../AnimationPlayer/EditorPlayer";
import {fileLoader} from "./FileLoader";
import {findParentContent, findParentPanel, HHSideBar} from "hhpanel";
import {sceneViewManager} from "./SceneViewManager";
import {SVGFiles} from "../Utilities/Svgs";

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
    storeId: number = null;

    inited: boolean = false;
    timeline: HHTimeline = null;

    createCanvasContainer() {
        this.canvasContainer = document.createElement("div")
        this.canvasContainer.id = "CanvasContainer"
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
        this.canvas.className = "SceneViewCanvas"
        this.canvas.setAttribute("resize", "true")
        this.canvas.style.padding = "0"
        this.canvas.style.margin = "0"
        this.canvas.classList.add("dockable")
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

    OnKeyDown(e:KeyboardEvent){
        if(e.key == "Escape"){
            this.endOfDrawingShape(this.currentShapeDrawer)
        }
    }

    setupEventsAndCreateFirstTrack() {
        window.addEventListener("resize", this.OnResize.bind(this))
        window.addEventListener("keydown", this.OnKeyDown.bind(this))

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.canvasContainer)

        EventBus.getInstance().on(EventNames.DRAWSHAPEBEGINS, this.beginToDrawShape.bind(this))
        EventBus.getInstance().on(EventNames.DRAWSHAPEENDS, this.endOfDrawingShape.bind(this))

        this.canvas.addEventListener("mousedown", this.onMouseDown.bind(this))
        this.canvas.addEventListener("mousemove", this.onMouseMove.bind(this))
        this.canvas.addEventListener("mouseup", this.onMouseUp.bind(this))
        this.canvas.addEventListener("dblclick", this.onDbClick.bind(this))

        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            _this.timeline = document.createElement("hh-timeline") as HHTimeline
            _this.canvasContainer.insertBefore(_this.timeline, _this.canvas)

            let i18n = (window as any).i18n;
            i18n.ExecuteAfterInited(()=>{
                _this.timeline.contextMenu.setItems([
                    {
                        itemName: i18n.t("contextmenu.mergecells"),
                        onclick: _this.timeline.mergeCells.bind(_this.timeline)
                    },
                    {
                        itemName: i18n.t("contextmenu.createNewTrack"),
                        onclick: function(e){
                            _this.createNewTrack(_this.timeline)
                        }
                    },
                    {
                        itemName: i18n.t("contextmenu.markAsAnimationEnd"),
                        onclick: function(e){
                            _this.markAsAnimationEnd(_this.timeline)
                        }
                    }
                ])

                // If no layer in the store now, create a new track.
                let layerCount = huahuoEngine.GetCurrentStore().GetLayerCount()
                if(layerCount == 0)
                    _this.createNewTrack(_this.timeline)
                else
                    _this.timeline.reloadTracks()

                if(_this.animationPlayer == null){
                    _this.animationPlayer = new EditorPlayer(_this)
                    huahuoEngine.setActivePlayer(_this.animationPlayer)
                }
            })
        })
    }

    markAsAnimationEnd(timeline: HHTimeline){
        let titleTrack = timeline.getTrack(0)
        if(titleTrack == null)
        {
            Logger.error("No title bar??")
            return;
        }
        let cellId = titleTrack.getCurrentCellId()
        console.log("Set max animation frame as:" + cellId)

        let firstTrack = timeline.getTrack(1)
        if(firstTrack == null){
            Logger.error("No track in the timeline??")
            return;
        }

        firstTrack.getLayer().GetObjectStore().UpdateMaxFrameId(cellId, true)
    }

    createNewTrack(timeline){
        let eyeIcon = new Image()
        eyeIcon.src = SVGFiles.eyeSvg
        eyeIcon["onIconClicked"] = function(layer){
            let currentlyVisible = layer.GetIsVisible()
            if(currentlyVisible){ // Currently visible
                eyeIcon.src = SVGFiles.eyeSlashSvg
            }else{
                eyeIcon.src = SVGFiles.eyeSvg
            }

            layer.SetIsVisible(!currentlyVisible)
        }

        eyeIcon.onload = function(){
            timeline.reloadTracks()
        }

        timeline.reloadTracks()
        timeline.addNewTrack(null, [eyeIcon])
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

    connectedCallback() {
        if(!this.inited){
            this.style.width = "100%"
            this.style.height = "100%"

            this.createCanvasContainer()
            this.createCanvas()
            renderEngine2D.init(this.canvas);

            // (window as any).i18n.ExecuteAfterInited(this.createGizmos.bind(this)) // Need translate here!

            // this.drawCoordinate();
            this.setupEventsAndCreateFirstTrack()

            defaultShapeDrawer.onBeginToDrawShape(this.canvas)

            let _this = this
            huahuoEngine.ExecuteAfterInited(() => {
                _this.storeId = huahuoEngine.GetCurrentStore().GetStoreId()
                console.log("New store id:" + _this.storeId)

                if(!_this.animationPlayer){
                    _this.animationPlayer = new EditorPlayer(_this)
                    huahuoEngine.setActivePlayer(_this.animationPlayer)
                }
                _this.animationPlayer.storeId = _this.storeId
            })

            this.inited = true

            sceneViewManager.registerSceneView(this)

            huahuoEngine.ExecuteAfterInited(()=>{
                sceneViewManager.focusSceneView(_this)
            })

            // Refresh sidebar content.
            let sidebars = document.querySelectorAll("hh-sidebar")
            for(let sidebar of sidebars){
                (sidebar as HHSideBar).refreshDockables()
            }
        }
    }

    onDbClick(evt:MouseEvent){
        if (this.currentShapeDrawer && !this.isPlaying) {
            this.currentShapeDrawer.onDblClick(evt)
        }
    }

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

        if (this.currentShapeDrawer && !this.isPlaying) {
            this.currentShapeDrawer.onMouseDown(evt)
        }
    }

    resetDefaultShapeDrawer(){
        defaultShapeDrawer.onBeginToDrawShape(this.canvas)
    }

    get isPlaying():boolean{
        return null != this.animationPlayer && this.animationPlayer.isPlaying
    }

    onMouseMove(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.isPlaying) {
            this.currentShapeDrawer.onMouseMove(evt)
        }
    }

    onMouseUp(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.isPlaying) {
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
        if(window.getComputedStyle(this.parentElement).display == "none")
            return;

        // Find the panel
        let panel = findParentPanel(this.canvasContainer)

        if(panel == null)  // The scene view might has already been closed
            return;

        // TODO: Move this into HHPanel??
        let panelWidth = panel.clientWidth
        let panelHeight = panel.clientHeight
        let titleHeight = panel.querySelector(".title_tabs").clientHeight

        let timelineHeight = this.timeline?this.timeline.canvasScrollContainer.offsetHeight:0;
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

        if(this.gizmoContainer)
            this.gizmoContainer.style.top = this.canvas.offsetHeight + timelineHeight + "px"

        // Refresh sidebar content.
        let sidebars = document.querySelectorAll("hh-sidebar")
        for(let sidebar of sidebars){
            (sidebar as HHSideBar).refreshDockables()
        }
    }

    Redraw() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height)
        this.ctx.fillStyle = "white"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        let previousCanvas = renderEngine2D.setDefaultCanvas(this.canvas)
        renderEngine2D.clearBackground()
        if(previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)
    }

}

export {SceneView}