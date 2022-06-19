import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS, paper} from "hhenginejs"

class ShapeScaleHandler extends ShapeTranslateMorphBase{

    protected lastPos: Vector2 = null

    constructor() {
        super();
        this.isDragging = false
    }

    beginMove(startPos) {
        super.beginMove(startPos);
        this.lastPos = startPos
    }

    dragging(newPos: Vector2) {
        super.dragging(newPos);

        if(this.isDragging && this.curObjs != null){


            for(let obj of this.curObjs){
                let center = obj.getPaperShape().bounds.center // current center
                let vec1 = this.lastPos.subtract(center)
                let vec2 = newPos.subtract(center)

                let scale = vec2.length()/vec1.length()
                obj.scale(scale)
                obj.update()
            }

            this.lastPos = newPos
        }
    }
}

let shapeScaleHandler = new ShapeScaleHandler()
export {shapeScaleHandler}