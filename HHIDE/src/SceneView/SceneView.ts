import {CustomElement, Logger} from "hhcommoncomponents"
import {EngineJS} from "hhenginejs"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {EngineAPI} from "../EngineAPI";

@CustomElement({
    selector: "hh-sceneview"
})
class SceneView extends HTMLElement{
    aspectRatio:number = 4/3  //  W:H = 4:3
    canvasContainer: HTMLDivElement
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D = null;

    currentShapeDrawer: BaseShapeDrawer = null;

    connectedCallback(){
        this.style.width = "100%"
        this.style.height = "100%"

        this.canvasContainer = document.createElement("div")
        this.canvasContainer.style.width = "100%"
        this.canvasContainer.style.height = "100%"
        this.canvasContainer.style.backgroundColor = "gray"

        this.appendChild(this.canvasContainer)

        this.canvas = document.createElement("canvas")
        this.canvasContainer.appendChild(this.canvas)

        this.ctx = this.canvas.getContext("2d")

        this.OnResize()
        window.addEventListener("resize", this.OnResize.bind(this))

        EventBus.getInstance().on(EventNames.DRAWSHAPEBEGINS, this.beginToDrawShape.bind(this))
        EventBus.getInstance().on(EventNames.DRAWSHAPEENDS, this.endOfDrawingShape.bind(this))

        this.canvas.addEventListener("mousedown", this.onMouseDown.bind(this))
        this.canvas.addEventListener("mousemove", this.onMouseMove.bind(this))
        this.canvas.addEventListener("mouseup", this.onMouseUp.bind(this))

        EngineJS.prototype.getInstance().init(this.canvas)
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

        let canvasWidth = containerWidth
        let canvasHeight = canvasWidth/this.aspectRatio

        if(canvasHeight > containerHeight){
            canvasHeight = containerHeight
            canvasWidth = canvasHeight * this.aspectRatio
        }

        this.canvas.width = canvasWidth
        this.canvas.height = canvasHeight
        this.canvas.style.position = "relative"
        this.canvas.style.left = (containerWidth - canvasWidth)/2 + "px"
        this.Redraw()
    }

    Redraw(){
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height)
        this.ctx.fillStyle = "white"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)
    }

}

export {SceneView}