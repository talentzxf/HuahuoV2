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
                let proposedNewPosition = obj.getPaperShape().position.add(offset)

                // TODO: check whether the position is acceptable or need some modification

                // obj.position = proposedNewPosition
                console.log("Proposing:" + proposedNewPosition.x + "," + proposedNewPosition.y)
                obj.getPaperShape().position = proposedNewPosition
                console.log("After set position:" + obj.getPaperShape().position.x + "," + obj.getPaperShape().position.y)
                obj.store({segments: true, position: true})
                obj.update({updateShape:false, updateBoundingBox:true})
            }
        }
    }
}

let shapeTranslateHandler = new ShapeTranslateHandler()

export {shapeTranslateHandler}