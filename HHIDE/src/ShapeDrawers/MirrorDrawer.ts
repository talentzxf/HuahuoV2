import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {SVGFiles} from "../Utilities/Svgs";
import {Vector2} from "hhcommoncomponents";
import {MirrorShapeJS, huahuoEngine} from "hhenginejs";
import {EventBus, EventNames} from "../Events/GlobalEvents";

// Maybe we should just add a mirror component in the lineshape ???
class MirrorDrawer extends BaseShapeDrawer {
    name = "Mirror"
    imgCss = SVGFiles.mirrorImg

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

        this.tempShape = new MirrorShapeJS()
        this.tempShape.setStartPoint(this.startPosition)
        this.tempShape.setEndPoint(this.startPosition)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if(this.isDrawing){
            let currentPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

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

            _this.addShapeToCurrentLayer(_this.tempShape)
        })
    }
}

export {MirrorDrawer}