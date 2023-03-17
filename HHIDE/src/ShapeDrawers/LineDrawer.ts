import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2} from "hhcommoncomponents";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {LineShapeJS} from "hhenginejs"
import {huahuoEngine} from "hhenginejs"
import {setPrompt} from "../init";

class LineDrawer extends BaseShapeDrawer {
    name = 'Line'
    imgClass = "fas fa-slash"

    tempShape

    pressingShift: boolean = false

    startPosition = new Vector2()

    constructor() {
        super();
        document.body.addEventListener("keydown", this.onKeyDown.bind(this))
        document.body.addEventListener("keyup", this.onKeyUp.bind(this))
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"

        setPrompt(i18n.t("statusbar.drawLine"))
    }

    onKeyDown(e: KeyboardEvent) {
        if (e.shiftKey) {
            this.pressingShift = true
        }
    }

    onKeyUp(e: KeyboardEvent) {
        if (!e.shiftKey) {
            this.pressingShift = false
        }
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);
        this.startPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.isDrawing = true

        this.tempShape = new LineShapeJS()
        this.tempShape.setStartPoint(this.startPosition)
        this.tempShape.setEndPoint(this.startPosition)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if (this.isDrawing) {
            let currentPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

            if (this.pressingShift) {
                let offset = currentPos.subtract(this.startPosition)
                if (Math.abs(offset.x) > Math.abs(offset.y)) {
                    currentPos.y = this.startPosition.y
                } else {
                    currentPos.x = this.startPosition.x
                }
            }

            this.tempShape.setEndPoint(currentPos)
            this.tempShape.update(true)
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            _this.isDrawing = false
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)

            _this.addShapeToCurrentLayer(_this.tempShape)
        })
    }
}

export {LineDrawer}