import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"
import {undoManager} from "../RedoUndo/UndoManager";
import {ShapeMoveCommand} from "../RedoUndo/ShapeMoveCommand";
import {BaseShapeJS} from "hhenginejs"

class ShapeTranslateHandler extends ShapeTranslateMorphBase
{
    protected lastPos: Vector2 = null

    private pressingShift: boolean = false

    private startPosMap: Map<BaseShapeJS, Vector2> = new Map
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

        this.startPosMap.clear()
        for(let obj of this.curObjs){
            this.startPosMap.set(obj, obj.position)
        }
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

                if(obj.getFollowCurve && obj.getFollowCurve()){
                    let followingCurve = obj.getFollowCurve()
                    proposedNewPosition = followingCurve.getGlobalNearestPoint(proposedNewPosition)
                }

                obj.position = proposedNewPosition
                obj.store()
            }
        }
    }

    endMove() {
        for(let obj of this.curObjs){
            let prevPosition = this.startPosMap.get(obj)
            let command = new ShapeMoveCommand(obj, prevPosition, obj.position)
            undoManager.PushCommand(command)
        }
    }
}

let shapeTranslateHandler = new ShapeTranslateHandler()

export {shapeTranslateHandler}