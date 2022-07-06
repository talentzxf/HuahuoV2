import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS, paper} from "hhenginejs"

class ShapeScaleHandler extends ShapeTranslateMorphBase{

    protected lastPos: Vector2 = null
    protected originalScaleMap: Map<BaseShapeJS, Vector2> = new Map();

    constructor() {
        super();
        this.isDragging = false
    }

    beginMove(startPos) {
        super.beginMove(startPos);
        this.lastPos = startPos

        this.originalScaleMap.clear()
        for(let shape of this.curObjs){
            this.originalScaleMap.set(shape, shape.scaling)
        }
    }

    dragging(newPos: Vector2) {
        super.dragging(newPos);

        if(this.isDragging && this.curObjs != null){
            for(let obj of this.curObjs){
                let center = obj.position
                let vec1 = this.startPos.subtract(center)
                let vec2 = newPos.subtract(center)

                let scale = vec2.length()/vec1.length()
                obj.scaling = this.originalScaleMap.get(obj).multiply(scale)

                obj.store()
            }

            this.lastPos = newPos
        }
    }
}

let shapeScaleHandler = new ShapeScaleHandler()
export {shapeScaleHandler}