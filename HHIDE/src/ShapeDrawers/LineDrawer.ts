import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2} from "../../../HHCommonComponents/src/Math/Vector2";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {LineShape} from "hhenginejs"
import {ShapeStoreManager, Layer} from "hhenginejs"

class LineDrawer extends BaseShapeDrawer {
    name = 'Line'
    imgClass = "fas fa-slash"

    tempShape = new LineShape()

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

        this.isDrawing = false
        EventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, this)

        let currentLayer = ShapeStoreManager.getInstance().getStore().getCurrentLayer()
        currentLayer.addShape(this.tempShape)

        this.tempShape = new LineShape();
    }
}

export {LineDrawer}