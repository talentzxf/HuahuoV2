import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"

class ShapeTranslateHandler extends ShapeTranslateMorphBase
{
    protected lastPos: Vector2 = null

    private pressingShift: boolean = false
    constructor() {
        super();
        this.isDragging = false

        document.body.addEventListener("keydown", this.onKeyDown.bind(this))
        document.body.addEventListener("keyup", this.onKeyUp.bind(this))


    }

    onKeyDown(e:KeyboardEvent){
        if(e.shiftKey){
            this.pressingShift = true
        }
    }

    onKeyUp(e:KeyboardEvent){
        if(!e.shiftKey){
            this.pressingShift = false
        }
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
                // TODO: check whether the position is acceptable or need some modification
                if(this.pressingShift){
                    if(Math.abs(offset.x) > Math.abs(offset.y)){ // X dominant
                        offset.y = 0.0
                    }else{
                        offset.x = 0.0
                    }
                }

                let proposedNewPosition = obj.position.add(offset)
                // obj.position = proposedNewPosition
                obj.position = proposedNewPosition
                obj.store()
            }
        }
    }
}

let shapeTranslateHandler = new ShapeTranslateHandler()

export {shapeTranslateHandler}