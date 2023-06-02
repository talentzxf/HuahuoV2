import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS, paper} from "hhenginejs"
import {ShapeScaleCommand} from "../RedoUndo/ShapeScaleCommand";
import {undoManager} from "../RedoUndo/UndoManager";

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

    getNewScale(obj, scale){
        return this.originalScaleMap.get(obj).multiply(scale)
    }

    dragging(newPos: Vector2) {
        super.dragging(newPos);

        if(this.isDragging && this.curObjs != null){
            for(let obj of this.curObjs){
                let center = obj.shapePosition
                let vec1 = this.startPos.subtract(center)
                let vec2 = newPos.subtract(center)

                let scale = vec2.length()/vec1.length()
                obj.scaling = this.getNewScale(obj, scale)

                obj.store()
            }

            this.lastPos = newPos
        }
    }

    endMove() {
        super.endMove();

        for(let obj of this.curObjs){
            let command = new ShapeScaleCommand(obj, this.originalScaleMap.get(obj), obj.scaling)
            undoManager.PushCommand(command)
        }
    }
}

class ShapeHorizontalScaleHandler extends ShapeScaleHandler{
    override getNewScale(obj, scale){
        let originalScale = this.originalScaleMap.get(obj)
        return originalScale.multiply(new paper.Point(scale, 1.0))
    }
}

class ShapeVerticalScaleHandler extends ShapeScaleHandler{
    override getNewScale(obj, scale): any {
        let originalScale = this.originalScaleMap.get(obj)
        return originalScale.multiply(new paper.Point(1.0, scale))
    }
}

let shapeScaleHandler = new ShapeScaleHandler()
let shapeHorizontalScaleHandler = new ShapeHorizontalScaleHandler()
let shapeVerticalScaleHandler = new ShapeVerticalScaleHandler()
export {shapeScaleHandler, shapeHorizontalScaleHandler, shapeVerticalScaleHandler, ShapeScaleHandler}