import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {huahuoEngine, CurveShapeJS} from "hhenginejs";
import {EventBus, EventNames} from "../Events/GlobalEvents";

class CurveDrawer extends BaseShapeDrawer {
    name = "Curve"
    imgClass = "fas fa-bezier-curve"

    curvePath: CurveShapeJS

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        this.curvePath = new CurveShapeJS()

        this.curvePath.initComponents()

        let point = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.curvePath.addPoint(point)

        this.isDrawing = true
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);

        if (this.isDrawing) {
            let newPoint = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
            this.curvePath.addPoint(newPoint)
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        this.curvePath.endDrawingCurve()

        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            _this.isDrawing = false

            EventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)
            _this.addShapeToCurrentLayer(_this.curvePath)
        })
    }
}

export {CurveDrawer}