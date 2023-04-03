import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {SVGFiles} from "../Utilities/Svgs";
import {StarMirrorShapeJS, huahuoEngine} from "hhenginejs"
import {setPrompt} from "../init";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";

class StarMirrorDrawer extends BaseShapeDrawer{
    name = "StarMirror"
    imgCss = SVGFiles.rotationalSymmetry

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"
        setPrompt(i18n.t("statusBar.drawStarMirror"))
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        let currentPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        let tempShape = new StarMirrorShapeJS()
        tempShape.update(true)
        tempShape.position = currentPosition

        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)
            _this.addShapeToCurrentLayer(tempShape)
        })
    }
}

export {StarMirrorDrawer}