import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"
import {undoManager} from "../RedoUndo/UndoManager";
import {ShapeMoveCommand} from "../RedoUndo/ShapeMoveCommand";
import {BaseShapeJS} from "hhenginejs"
import {FollowCurveComponent} from "hhenginejs";

function getFollowCurveComponentFromBaseShape(shape: BaseShapeJS): FollowCurveComponent{
    let component:FollowCurveComponent = shape.getComponentByTypeName("FollowCurveComponent")
    if(!component)
        return null
    return component
}

class ShapeTranslateHandler extends ShapeTranslateMorphBase
{
    private lastPos: Vector2 = null
    private pressingShift: boolean = false

    // Map from shape->local position of the mouse.
    // After translation, local position should still remain unchanged.
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
            this.startPosMap.set(obj, obj.globalToLocal(startPos))
        }
    }

    // The pos is already in world space.
    dragging(newPos: Vector2) {
        if(this.isDragging && this.curObjs != null){

            for(let obj of this.curObjs){
                console.log("local position:" + this.startPosMap.get(obj))
                let prevGlobalPosition = obj.localToGlobal(this.startPosMap.get(obj))
                console.log("prevGlobalPosition:" + prevGlobalPosition.x + "," + prevGlobalPosition.y)
                let newGlobalPosition = newPos
                let offset = new paper.Point(newGlobalPosition.x - prevGlobalPosition.x, newGlobalPosition.y - prevGlobalPosition.y)

                let mouseOffset = new paper.Point(newPos.x - this.lastPos.x , newPos.y - this.lastPos.y)
                // TODO: check whether the position is acceptable or need some modification
                if(this.pressingShift){
                    if(Math.abs(mouseOffset.x) > Math.abs(mouseOffset.y)){ // X dominant
                        console.log("X dorminant")
                        offset.y = 0.0
                    }else{
                        console.log("Y dorminant")
                        offset.x = 0.0
                    }
                }

                let proposedNewPosition = obj.position.add(offset)


                let followCurveComponent = getFollowCurveComponentFromBaseShape(obj)
                if(followCurveComponent != null){
                    let followingCurve = followCurveComponent.getFollowingTargetShape()

                    if(followingCurve){
                        // If it's following a curve, no need to update its local position.
                        proposedNewPosition = followingCurve.getGlobalNearestPoint(proposedNewPosition)

                        let length = followingCurve.getGlobalOffsetOf(proposedNewPosition)
                        followCurveComponent.lengthRatio = length / followingCurve.length()
                    }
                }

                obj.position = proposedNewPosition

                obj.store()
            }

            this.lastPos = newPos
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