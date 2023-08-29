import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {SVGFiles} from "../Utilities/Svgs";
import {paper} from "hhenginejs"
import {itemSelectable} from "./ShapeSelector";
import {NailComponent, NailShapeJS} from "hhenginejs";
import {getNailManager, isInheritedFromClzName} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {EditorShapeProxy} from "./EditorShapeProxy";

class NailDrawer extends BaseShapeDrawer {
    name = "Nail"
    imgCss = SVGFiles.nailSvg

    hitOptions = {
        segments: false,
        stroke: false,
        fill: true,
        handles: false,
        tolerance: 5
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        let scrX = evt.offsetX
        let scrY = evt.offsetY
        let hitPoint = BaseShapeDrawer.getWorldPosFromView(scrX, scrY)

        let hitShapes = []
        let hitResultArray = paper.project.hitTestAll(hitPoint, this.hitOptions)

        for (let hitResult of hitResultArray) {
            if (hitResult) {
                let hitItem = hitResult.item
                if (itemSelectable(hitResult.item)) {
                    let shape = hitItem.data.meta
                    // if (isInheritedFromClzName(shape, "BaseSolidShape")){
                    if (isInheritedFromClzName(shape, "BaseShapeJS")){
                        if(shape.isMirage){
                            HHToast.warn(i18n.t("toast.nailMirageError"))
                        }else{
                            hitShapes.push(shape)
                        }
                    }
                }
            }
        }

        if (hitShapes.length <= 0)
            return

        for (let i = 0; i < hitShapes.length; i++) {
            for (let j = i + 1; j < hitShapes.length; j++) {
                if (!getNailManager().checkDuplication(hitShapes[i], hitShapes[j])) {
                    HHToast.warn(i18n.t("toast.nailDuplicated"))
                    return
                }
            }
        }

        let nail = EditorShapeProxy.CreateProxy(new NailShapeJS())
        for (let shape of hitShapes) {
            nail.addShape(shape, new paper.Point(hitPoint.x, hitPoint.y))
            let nailComponent = shape.getComponentByTypeName("NailComponent")
            if (nailComponent == null) {
                nailComponent = new NailComponent()
                shape.addComponent(nailComponent)
            }

            nailComponent.addNail(nail)
        }

        if (nail) {
            this.addShapeToCurrentLayer(nail)
            nail.update(true)
        }
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        canvas.style.cursor = "url(\"" + SVGFiles.hammerNailSvg + "\") 0 32, pointer"
    }
}

export {NailDrawer}