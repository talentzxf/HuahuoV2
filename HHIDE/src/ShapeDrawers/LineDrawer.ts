import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2} from "hhcommoncomponents";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {LineShapeJS} from "hhenginejs"
import {huahuoEngine} from "hhenginejs"

class LineDrawer extends BaseShapeDrawer {
    name = 'Line'
    imgClass = "fas fa-slash"

    tempShape = new LineShapeJS()

    startPosition = new Vector2()
    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"
    }

    onMouseDown(evt:MouseEvent) {
        super.onMouseDown(evt);
        this.startPosition = this.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.isDrawing = true

        this.tempShape.setStartPoint(this.startPosition)
        this.tempShape.setEndPoint(this.startPosition)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if(this.isDrawing){
            let currentPos = this.getWorldPosFromView(evt.offsetX, evt.offsetY)

            this.tempShape.setEndPoint(currentPos)
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

            _this.tempShape = new LineShapeJS();
        })
    }
}

export {LineDrawer}