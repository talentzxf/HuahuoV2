import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {paper} from "hhenginejs"
import {Vector2} from "hhcommoncomponents"
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";

class ShapeTranslateHandler extends ShapeTranslateMorphBase
{
    constructor() {
        super();
        this.isDragging = false
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