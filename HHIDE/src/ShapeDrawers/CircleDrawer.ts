import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {CircleShapeJS} from "hhenginejs"
import {huahuoEngine} from "hhenginejs"
import {Vector2} from "hhcommoncomponents";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {undoManager} from "../RedoUndo/UndoManager";
import {CreateShapeCommand} from "../RedoUndo/CreateShapeCommand";

class CircleDrawer extends BaseShapeDrawer {
    name = "Circle"
    imgClass = "fas fa-circle"
    tempShape = null

    startPosition = new Vector2()

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);
        this.startPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.isDrawing = true

        this.tempShape = new CircleShapeJS()
        this.tempShape.setCenter(this.startPosition)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if (this.isDrawing) {
            let endPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

            let radius = endPosition.distance(this.startPosition)
            this.tempShape.setRadius(radius)
            this.tempShape.update(true)
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            _this.isDrawing = false
            EventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)
            _this.addShapeToCurrentLayer(_this.tempShape)
        })
    }

}

export {CircleDrawer}