import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {CircleShapeJS} from "hhenginejs"
import {huahuoEngine} from "hhenginejs"
import {Vector2} from "hhcommoncomponents";
import {EventBus, EventNames} from "../Events/GlobalEvents";

class CircleDrawer extends BaseShapeDrawer{
    name = "Circle"
    imgClass = "fas fa-circle"
    tempShape = new CircleShapeJS()

    startPosition = new Vector2()
    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"
    }

    onMouseDown(evt:MouseEvent) {
        super.onMouseDown(evt);
        this.startPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.isDrawing = true

        this.tempShape.setCenter(this.startPosition)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if(this.isDrawing){
            let endPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

            let radius = endPosition.distance(this.startPosition)
            this.tempShape.setRadius(radius)
            this.tempShape.update()
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            _this.isDrawing = false
            EventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)

            let currentLayer = huahuoEngine.GetCurrentLayer()
            currentLayer.addShape(this.tempShape)

            _this.tempShape = new CircleShapeJS();
        })
    }

}

export {CircleDrawer}