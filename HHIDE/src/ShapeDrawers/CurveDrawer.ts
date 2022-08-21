import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {CurveShapeJS} from "hhenginejs";

class CurveDrawer extends BaseShapeDrawer{
    name = "Curve"
    imgClass = "fas fa-bezier-curve"

    curvePath:CurveShapeJS = new CurveShapeJS()

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        let point = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.curvePath.addPoint(point)

        this.isDrawing = true
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);

        if(this.isDrawing){
            let newPoint = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
            this.curvePath.addPoint(newPoint)
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        this.curvePath.simplify(10)

        this.isDrawing = false

        this.curvePath = new CurveShapeJS()
    }
}

export {CurveDrawer}