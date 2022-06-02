import {CustomElement, Logger} from "hhcommoncomponents"
import {EngineJS} from "hhenginejs"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {HHTimeline, TimelineEventNames, TimelineTrack} from "hhtimeline"
import {ShapeStoreManager} from "hhenginejs"
import { ResizeObserver } from 'resize-observer';

@CustomElement({
    selector: "hh-sceneview"
})
class SceneView extends HTMLElement{
    canvasContainer: HTMLDivElement
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D = null;

    currentShapeDrawer: BaseShapeDrawer = null;

    connectedCallback(){
        this.style.width = "100%"
        this.style.height = "100%"

        this.canvasContainer = document.createElement("div")
        // this.canvasContainer.style.width = "100%"
        // this.canvasContainer.style.height = "100%"
        this.canvasContainer.style.padding = "0"
        this.canvasContainer.style.margin = "0"
        this.canvasContainer.style.backgroundColor = "gray"
        this.canvasContainer.style.flexBasis = "100%"
        this.canvasContainer.style.overflowY = "auto"

        this.appendChild(this.canvasContainer)

        this.canvas = document.createElement("canvas")
        this.canvas.setAttribute("resize", "true")
        this.canvas.style.padding = "0"
        this.canvas.style.margin = "0"
        this.canvasContainer.appendChild(this.canvas)

        this.ctx = this.canvas.getContext("2d")

        EngineJS.prototype.getInstance().init(this.canvas)

        window.addEventListener("resize", this.OnResize.bind(this))

        let resizeObserver = new ResizeObserver( this.OnResize.bind(this) )
        resizeObserver.observe(this.canvasContainer)

        EventBus.getInstance().on(EventNames.DRAWSHAPEBEGINS, this.beginToDrawShape.bind(this))
        EventBus.getInstance().on(EventNames.DRAWSHAPEENDS, this.endOfDrawingShape.bind(this))

        this.canvas.addEventListener("mousedown", this.onMouseDown.bind(this))
        this.canvas.addEventListener("mousemove", this.onMouseMove.bind(this))
        this.canvas.addEventListener("mouseup", this.onMouseUp.bind(this))

        let timeline:HHTimeline = document.querySelector("hh-timeline")
        timeline.addEventListener(TimelineEventNames.NEWTRACKADDED, this.onNewTrackAdded.bind(this))

        timeline.addNewTrack()
    }

    onNewTrackAdded(trackEvent: CustomEvent){
        let track = trackEvent.detail.targetObj
        ShapeStoreManager.getInstance().getStore().createLayer(track.getId())
    }

    onMouseDown(evt:MouseEvent){
        if(this.currentShapeDrawer){
            this.currentShapeDrawer.onMouseDown(evt)
        }
    }

    onMouseMove(evt:MouseEvent){
        if(this.currentShapeDrawer){
            this.currentShapeDrawer.onMouseMove(evt)
        }
    }

    onMouseUp(evt: MouseEvent){
        if(this.currentShapeDrawer){
            this.currentShapeDrawer.onMouseUp(evt)
        }
    }

    beginToDrawShape(shapeDrawer: BaseShapeDrawer){
        this.currentShapeDrawer = shapeDrawer;
        this.currentShapeDrawer.onBeginToDrawShape(this.canvas)
    }

    endOfDrawingShape(shapeDrawer: BaseShapeDrawer){
        this.currentShapeDrawer = null;
        this.canvas.style.cursor = "default"
    }

    OnResize(){
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
        this.canvas.style.left = (containerWidth - canvasWidth)/2 + "px"
        this.canvas.style.top = (containerHeight - canvasHeight)/2 + "px"
        this.Redraw()
    }

    Redraw(){
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height)
        this.ctx.fillStyle = "white"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        EngineJS.prototype.getInstance().clearBackground()
    }

}

export {SceneView}