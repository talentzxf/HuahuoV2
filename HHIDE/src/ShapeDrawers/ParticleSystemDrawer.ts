import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {SVGFiles} from "../Utilities/Svgs";
import {Vector2} from "hhcommoncomponents";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {ParticleSystemJS} from "hhenginejs";
import {huahuoEngine} from "hhenginejs";
import {EditorShapeProxy} from "./EditorShapeProxy";

class ParticleSystemDrawer extends BaseShapeDrawer {
    name = "Particles"
    imgCss = SVGFiles.fireworksSvg

    startPosition = new Vector2()

    tempShape

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        canvas.style.cursor = "crosshair"
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        this.startPosition = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        this.isDrawing = true

        this.tempShape = EditorShapeProxy.CreateProxy(new ParticleSystemJS())
        this.tempShape.setStartPoint(this.startPosition)
        this.tempShape.setEndPoint(this.startPosition)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if(this.isDrawing){
            let currentPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

            this.tempShape.setEndPoint(currentPos)
            this.tempShape.update(true)
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        let _this = this

        huahuoEngine.ExecuteAfterInited(()=>{
            this.isDrawing = false
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, this)
            _this.addShapeToCurrentLayer(_this.tempShape)

            let particleSystemRender = huahuoEngine.produceObject("ParticleSystemRenderer")
            particleSystemRender.addParticles(huahuoEngine.produceObject("Particles"))

            _this.tempShape.addComponent(particleSystemRender)

            _this.tempShape.update(true)
        })
    }
}

export {ParticleSystemDrawer}