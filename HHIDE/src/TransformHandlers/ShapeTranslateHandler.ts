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
        if(this.isDragging && this.curObj != null){
            let offset = newPos.subtract(this.lastPos)

            let proposedNewPosition = this.curObj.position.add(offset)

            // TODO: check whether the position is acceptable or need some modification

            this.curObj.position = proposedNewPosition
            this.lastPos = newPos

            this.curObj.update()

            super.dragging(newPos)
        }
    }
}

let shapeTranslateHandler = new ShapeTranslateHandler()

export {shapeTranslateHandler}