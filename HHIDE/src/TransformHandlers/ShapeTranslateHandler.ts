import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"

class ShapeTranslateHandler extends ShapeTranslateMorphBase
{
    protected lastPos: Vector2 = null
    constructor() {
        super();
        this.isDragging = false
    }

    beginMove(startPos) {
        super.beginMove(startPos);
        this.lastPos = startPos
    }

    // The pos is already in world space.
    dragging(newPos: Vector2) {
        if(this.isDragging && this.curObjs != null){
            let offset = newPos.subtract(this.lastPos)
            this.lastPos = newPos

            for(let obj of this.curObjs){
                let proposedNewPosition = obj.position.add(offset)

                // TODO: check whether the position is acceptable or need some modification

                obj.position = proposedNewPosition
                obj.update()
            }
        }
    }
}

let shapeTranslateHandler = new ShapeTranslateHandler()

export {shapeTranslateHandler}