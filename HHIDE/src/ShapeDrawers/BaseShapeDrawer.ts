import {Logger, Vector2} from "hhcommoncomponents";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {renderEngine2D} from "hhenginejs"
import {BaseShapeJS} from "hhenginejs";
import {LayerUtils} from "../SceneView/Layer";

class BaseShapeDrawer {
    name = "unknown_shape"
    imgClass = "unknown_img"
    imgCss: string = null

    isDrawing = false
    _isSelected = false

    canvasWidth = -1;
    canvasHeight = -1;

    get isSelected() {
        return this._isSelected
    }

    set isSelected(val) {
        this._isSelected = val
    }

    isDefaultDrawer() {
        return false
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        this.canvasWidth = canvas.width
        this.canvasHeight = canvas.height
    }

    onMouseDown(evt: MouseEvent) {
    }

    static getWorldPosFromView(x, y): Vector2 {
        return renderEngine2D.getWorldPosFromView(x, y)
    }

    addShapeToCurrentLayer(shape: BaseShapeJS) {
        LayerUtils.addShapeToCurrentLayer(shape)
    }

    onMouseUp(evt: MouseEvent) {
    }

    onMouseMove(evt: MouseEvent) {

    }

    onDblClick(evt: MouseEvent) {

    }
}

export {BaseShapeDrawer}