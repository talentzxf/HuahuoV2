import {CustomElement, Logger} from "hhcommoncomponents"
import {renderEngine2D, huahuoEngine} from "hhenginejs"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {HHTimeline} from "hhtimeline"
import {ResizeObserver} from 'resize-observer';
import {defaultShapeDrawer} from "../ShapeDrawers/Shapes";
import {EditorPlayer} from "./EditorPlayer";
import {fileLoader} from "./FileLoader";
import {findParentContent, findParentPanel} from "../Utilities/PanelUtilities";
import {sceneViewManager} from "./SceneViewManager";

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
    animationPlayer: EditorPlayer = null;
    storeId: number = null;

    inited: boolean = false;

    static eyeSvg:string = "data:image/svg+xml,%3C%3Fxml version='1.0' encoding='utf-8'%3F%3E%3C!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'%3E%3Csvg version='1.1' xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512' xmlns:xlink='http://www.w3.org/1999/xlink' enable-background='new 0 0 512 512'%3E%3Cg%3E%3Cg%3E%3Cpath d='m494.6,241.1l-50.1-47c-50.5-47.3-117.4-73.3-188.5-73.3-71.1,0-138,26-188.4,73.3l-50.1,47c-12.1,12.9-4.3,26.5 0,29.8l50.1,47c50.4,47.3 117.3,73.3 188.4,73.3 71.1,0 138-26 188.4-73.3l50.1-47c4.7-3.9 12.2-17.6 0.1-29.8zm-238.6,74.9c-33.1,0-60-26.9-60-60 0-33.1 26.9-60 60-60 33.1,0 60,26.9 60,60 0,33.1-26.9,60-60,60zm-194.7-60l34.3-32.1c32-30 72-49.9 115.5-58.1-33.1,16.6-55.8,50.8-55.8,90.2 0,39.4 22.8,73.7 55.8,90.2-43.5-8.1-83.5-28.1-115.5-58.1l-34.3-32.1zm355.2,32.1c-32,30-72,50-115.5,58.1 33.1-16.6 55.8-50.8 55.8-90.2 0-39.4-22.8-73.6-55.8-90.2 43.5,8.1 83.5,28.1 115.5,58.1l34.3,32.1-34.3,32.1z'/%3E%3Cpath d='m256,235.2c-11.3,0-20.8,9.5-20.8,20.8 0,11.3 9.5,20.8 20.8,20.8 11.3,0 20.8-9.5 20.8-20.8 0-11.3-9.5-20.8-20.8-20.8z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E%0A";
    static eyeSlashSvg: string = "data:image/svg+xml,%0A%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 576 512'%3E%3Cpath d='M272.702 359.139c-80.483-9.011-136.212-86.886-116.93-167.042l116.93 167.042zM288 392c-102.556 0-192.092-54.701-240-136 21.755-36.917 52.1-68.342 88.344-91.658l-27.541-39.343C67.001 152.234 31.921 188.741 6.646 231.631a47.999 47.999 0 0 0 0 48.739C63.004 376.006 168.14 440 288 440a332.89 332.89 0 0 0 39.648-2.367l-32.021-45.744A284.16 284.16 0 0 1 288 392zm281.354-111.631c-33.232 56.394-83.421 101.742-143.554 129.492l48.116 68.74c3.801 5.429 2.48 12.912-2.949 16.712L450.23 509.83c-5.429 3.801-12.912 2.48-16.712-2.949L102.084 33.399c-3.801-5.429-2.48-12.912 2.949-16.712L125.77 2.17c5.429-3.801 12.912-2.48 16.712 2.949l55.526 79.325C226.612 76.343 256.808 72 288 72c119.86 0 224.996 63.994 281.354 159.631a48.002 48.002 0 0 1 0 48.738zM528 256c-44.157-74.933-123.677-127.27-216.162-135.007C302.042 131.078 296 144.83 296 160c0 30.928 25.072 56 56 56s56-25.072 56-56l-.001-.042c30.632 57.277 16.739 130.26-36.928 171.719l26.695 38.135C452.626 346.551 498.308 306.386 528 256z'/%3E%3C/svg%3E%3C!--%0AFont Awesome Free 5.2.0 by @fontawesome - https://fontawesome.com%0ALicense - https://fontawesome.com/license (Icons: CC BY 4.0, Fonts: SIL OFL 1.1, Code: MIT License)%0A--%3E"

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
        this.canvas.addEventListener("dblclick", this.onDbClick.bind(this))

        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            let timeline: HHTimeline = document.querySelector("hh-timeline")

            timeline.contextMenu.setItems([
                {
                    itemName: "Merge Selected Cells",
                    onclick: timeline.mergeCells.bind(timeline)
                },
                {
                    itemName: "Create New Track",
                    onclick: function(e){
                        _this.createNewTrack(timeline)
                    }
                }
            ])

            // If no layer in the store now, create a new track.
            let layerCount = huahuoEngine.GetCurrentStore().GetLayerCount()
            if(layerCount == 0)
                _this.createNewTrack(timeline)
        })
    }

    createNewTrack(timeline){
        let eyeIcon = new Image()
        eyeIcon.src = SceneView.eyeSvg
        eyeIcon["onIconClicked"] = function(layer){
            let currentlyVisible = layer.GetIsVisible()
            if(currentlyVisible){ // Currently visible
                eyeIcon.src = SceneView.eyeSlashSvg
            }else{
                eyeIcon.src = SceneView.eyeSvg
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
        if(!this.inited){
            this.style.width = "100%"
            this.style.height = "100%"

            this.createCanvasContainer()
            this.createCanvas()
            renderEngine2D.init(this.canvas)

            this.createGizmos();
            this.setupEventsAndCreateFirstTrack()

            this.animationPlayer = new EditorPlayer(this)
            defaultShapeDrawer.onBeginToDrawShape(this.canvas)

            let _this = this
            huahuoEngine.ExecuteAfterInited(() => {
                _this.storeId = huahuoEngine.GetCurrentStore().GetStoreId()
                console.log("New store id:" + _this.storeId)
            })

            this.inited = true

            sceneViewManager.registerSceneView(this)
        }
    }

    onDbClick(evt:MouseEvent){
        if (this.currentShapeDrawer && !this.animationPlayer.isPlaying) {
            this.currentShapeDrawer.onDblClick(evt)
        }
    }

    onMouseDown(evt: MouseEvent) {
        // Inform the panel to select my tab
        // Find the panel
        let panel = findParentPanel(this)
        // Find the content
        let content = findParentContent(this)
        let title = content.getTitle()
        panel.selectTab(title.tabIndex)

        if (this.currentShapeDrawer && !this.animationPlayer.isPlaying) {
            this.currentShapeDrawer.onMouseDown(evt)
        }
    }

    onMouseMove(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.animationPlayer.isPlaying) {
            this.currentShapeDrawer.onMouseMove(evt)
        }
    }

    onMouseUp(evt: MouseEvent) {
        if (this.currentShapeDrawer && !this.animationPlayer.isPlaying) {
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

        // TODO: Move this into HHPanel??
        let panelWidth = panel.clientWidth
        let panelHeight = panel.clientHeight
        let titleHeight = panel.querySelector(".title_tabs").clientHeight
        let contentHeight = panelHeight - titleHeight

//        let containerWidth = this.canvasContainer.clientWidth
//        let containerHeight = this.canvasContainer.clientHeight
//        Logger.debug("OnResize: ContainerWidth:" + containerWidth + ", ContainerHeight:" + containerHeight)

        let ele = this.canvasContainer.parentElement.parentElement.parentElement.parentElement
        Logger.debug("OnResize: ContainerParentWidth:" + ele.clientWidth + ", ContainerHeight:" + ele.clientHeight)

        let containerWidth = this.canvasContainer.clientWidth
        let containerHeight = contentHeight
        let margin = 15
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