import {MergableCommand, UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";

let commandName = "MoveShape"

class ShapeMoveCommand extends MergableCommand{
    targetShape:BaseShapeJS
    prevPos
    targetPos
    layer

    constructor(shape:BaseShapeJS, currentPos, targetPos) {
        super()
        this.targetShape = shape
        this.layer = shape.getLayer()
        this.prevPos = currentPos
        this.targetPos = targetPos
    }

    _DoCommand() {
        this.targetShape.position = this.targetPos
    }

    _UnDoCommand() {
        this.targetShape.position = this.prevPos
    }

    GetType(): string {
        return commandName;
    }

    MergeCommand(anotherCommand: MergableCommand): boolean { // Do we really need to merge the shape move command??
        // if(anotherCommand.GetType() == commandName){
        //     let moveCommand = anotherCommand as ShapeMoveCommand
        //     if(moveCommand){
        //         if(this.stackFrameEqual(anotherCommand)){
        //             // This is the same shape, in the same layer, in the same frameId, can merge the move command
        //             if(moveCommand.targetShape == this.targetShape &&
        //                 this.layer == moveCommand.layer){
        //                 this.targetPos = moveCommand.targetPos
        //                 return true
        //             }
        //         }
        //     }
        // }

        return false
    }

}

export {ShapeMoveCommand}