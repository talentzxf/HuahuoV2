import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {SVGFiles} from "../Utilities/Svgs";
import {paper} from "hhenginejs"
import {itemSelectable} from "./ShapeSelector";
import {BaseSolidShape, NailComponent} from "hhenginejs";
import {getNailManager, isInheritedFromClzName} from "hhenginejs";

class NailDrawer extends BaseShapeDrawer{
    name = "Nail"
    imgCss = SVGFiles.nailSvg

    hitOptions = {
        segments: false,
        stroke: false,
        fill: true,
        handles: false,
        tolerance: 5}

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        let scrX = evt.offsetX
        let scrY = evt.offsetY
        let hitPoint = BaseShapeDrawer.getWorldPosFromView(scrX, scrY)

        let hitShapes = []
        let hitResultArray = paper.project.hitTestAll(hitPoint, this.hitOptions)

        let nail = null
        for(let hitResult of hitResultArray){
            if(hitResult){
                let hitItem = hitResult.item
                if(itemSelectable(hitResult.item)){
                    let shape = hitItem.data.meta
                    if(isInheritedFromClzName(shape, "BaseSolidShape")){
                        hitShapes.push(shape)

                        if(nail == null){
                            nail = getNailManager().createNail()
                        }

                        nail.addShape(shape, hitPoint)

                        let nailComponent = shape.getComponentByTypeName("nail")
                        if(!nailComponent){
                            nailComponent = new NailComponent()
                            shape.addComponent(nailComponent)
                        }

                        nailComponent.addNail(nail)

                        shape.update(true)
                    }
                }
            }
        }
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        canvas.style.cursor = "url(\""+SVGFiles.hammerNailSvg+"\") 0 32, pointer"
    }
}

export {NailDrawer}