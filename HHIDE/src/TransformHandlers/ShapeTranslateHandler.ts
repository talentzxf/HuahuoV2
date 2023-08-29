import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {Vector2} from "hhcommoncomponents"
import {UndoableCommand, undoManager} from "../RedoUndo/UndoManager";
import {ShapeMoveCommand} from "../RedoUndo/ShapeMoveCommand";
import {BaseShapeJS} from "hhenginejs"
import {FollowCurveComponent} from "hhenginejs";
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {CommandArrayCommand} from "../RedoUndo/CommandArrayCommand";

let eps = 0.01

function getFollowCurveComponentFromBaseShape(shape: BaseShapeJS): FollowCurveComponent {
    if (shape instanceof BaseShapeJS) {
        let component: FollowCurveComponent = shape.getComponentByTypeName("FollowCurveComponent")
        if (component)
            return component
    }
    return null
}

class ShapeTranslateHandler extends ShapeTranslateMorphBase {
    private lastPos: Vector2 = null
    private pressingShift: boolean = false

    // Map from shape->local position of the mouse.
    // After translation, local position should still remain unchanged.
    private startPosMap: Map<BaseShapeJS, Vector2> = new Map

    private originalPosMap: Map<BaseShapeJS, Vector2> = new Map

    constructor() {
        super();
        this.isDragging = false

        document.body.addEventListener("keydown", this.onKeyDown.bind(this))
        document.body.addEventListener("keyup", this.onKeyUp.bind(this))
    }

    onKeyDown(e: KeyboardEvent) {
        if (e.shiftKey) {
            this.pressingShift = true
        }
    }

    onKeyUp(e: KeyboardEvent) {
        if (!e.shiftKey) {
            this.pressingShift = false
        }
    }

    beginMove(startPos) {
        super.beginMove(startPos);
        this.lastPos = startPos

        this.startPosMap.clear()

        this.originalPosMap.clear()

        for (let obj of this.curObjs) {
            this.startPosMap.set(obj, obj.globalToLocal(startPos))
            this.originalPosMap.set(obj, obj.position)
        }
    }

    // The pos is already in world space.
    dragging(newPos: Vector2) {
        if (this.isDragging && this.curObjs != null) {

            for (let obj of this.curObjs) {
                let prevGlobalPosition = obj.localToGlobal(this.startPosMap.get(obj))
                let newGlobalPosition = newPos
                let offset = new paper.Point(newGlobalPosition.x - prevGlobalPosition.x, newGlobalPosition.y - prevGlobalPosition.y)

                if (offset.length <= .001)
                    return

                if(obj["isLocked"] && obj.isLocked())
                    return

                let mouseOffset = new paper.Point(newPos.x - this.lastPos.x, newPos.y - this.lastPos.y)
                // TODO: check whether the position is acceptable or need some modification
                if (this.pressingShift) {
                    if (Math.abs(mouseOffset.x) > Math.abs(mouseOffset.y)) { // X dominant
                        console.log("X dorminant")
                        offset.y = 0.0
                    } else {
                        console.log("Y dorminant")
                        offset.x = 0.0
                    }
                }

                let proposedNewPosition = obj.position.add(offset)


                let followCurveComponent = getFollowCurveComponentFromBaseShape(obj)
                if (followCurveComponent != null) {
                    let followingCurve = followCurveComponent.getFollowingTargetShape()

                    if (followingCurve) {
                        // If it's following a curve, no need to update its local position.
                        proposedNewPosition = followingCurve.getGlobalNearestPoint(proposedNewPosition)

                        let length = followingCurve.getGlobalOffsetOf(proposedNewPosition)

                        let oldRatio = followCurveComponent.lengthRatio
                        let newRatio = length / followingCurve.length()

                        let setRatioCommand = new SetFieldValueCommand<number>(
                            followCurveComponent.setLengthRatio,
                            oldRatio,
                            newRatio
                        )

                        setRatioCommand.DoCommand()
                        undoManager.PushCommand(setRatioCommand)
                    }
                }

                console.log("ShapeTranslateHandler: Proposed New Position:" + proposedNewPosition)
                console.log("ShapeTranslateHandler: Before position:" + obj.position)
                obj.position = proposedNewPosition
                console.log("ShapeTranslateHandler: After position:" + obj.position)

                obj.store()
            }

            this.lastPos = newPos
        }
    }

    endMove() {
        let singleCommand = this.curObjs.size == 1

        let commandArray = new Array<UndoableCommand>()

        for (let obj of this.curObjs) {

            let prevPosition = this.originalPosMap.get(obj)

            if (obj.position.getDistance(prevPosition) >= eps) {
                let command = new ShapeMoveCommand(obj, prevPosition, obj.position)

                if(singleCommand)
                    undoManager.PushCommand(command)
                else{
                    commandArray.push(command)
                }
            }
        }

        if(!singleCommand){

            undoManager.PushCommand(new CommandArrayCommand(commandArray))
        }

    }
}

let shapeTranslateHandler = new ShapeTranslateHandler()

export {shapeTranslateHandler}