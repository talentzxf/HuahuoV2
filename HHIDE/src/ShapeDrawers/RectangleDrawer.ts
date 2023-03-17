import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {huahuoEngine, RectangleJS} from "hhenginejs"
import {Vector2} from "hhcommoncomponents"
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";

// This piece of code is almost the same as LineDrawer, maybe we should extract a common base class??
class RectangleDrawer extends BaseShapeDrawer{
    name = "Rectangle"
    imgClass = "fas fa-square"

    tempShape

    startPosition = new Vector2()

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        this.startPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.isDrawing = true

        this.tempShape = new RectangleJS()
        this.tempShape.setStartPoint(this.startPosition)
        this.tempShape.setEndPoint(this.startPosition)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if(this.isDrawing){
            let currentPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

            this.tempShape.setEndPoint(currentPos)
            this.tempShape.update(true)
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);
        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            _this.isDrawing = false
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)

            _this.addShapeToCurrentLayer(_this.tempShape)
        })
    }
}

export {RectangleDrawer}