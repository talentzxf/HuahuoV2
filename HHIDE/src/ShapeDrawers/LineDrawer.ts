import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2} from "../Math/Vector2";
import {EventBus, EventNames} from "../Events/GlobalEvents";

class LineDrawer extends BaseShapeDrawer {
    name = 'Line'
    imgClass = "fas fa-slash"

    startPosition = new Vector2()
    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"
    }

    onMouseDown(evt:MouseEvent) {
        super.onMouseDown(evt);
        this.startPosition = this.getRelativePosition(evt.clientX, evt.clientY)
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        EventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, this)
    }
}

export {LineDrawer}