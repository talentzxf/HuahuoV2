import {MergableCommand, UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";

let commandName = "MoveShape"

class ShapeMoveCommand implements MergableCommand{
    targetShape:BaseShapeJS
    prevPos
    targetPos

    constructor(shape:BaseShapeJS, currentPos, targetPos) {
        this.targetShape = shape
        this.prevPos = currentPos
        this.targetPos = targetPos
    }

    DoCommand() {
        this.targetShape.position = this.targetPos
        this.targetShape.store()
    }

    UnDoCommand() {
        this.targetShape.position = this.prevPos
        this.targetShape.store()
    }

    GetType(): string {
        return commandName;
    }

    MergeCommand(anotherCommand: MergableCommand): boolean {
        if(anotherCommand.GetType() == commandName){
            let moveCommand = anotherCommand as ShapeMoveCommand
            if(moveCommand){
                if(moveCommand.targetShape == this.targetShape){ // This is the same shape, can merge the move command
                    this.targetPos = moveCommand.targetPos
                    return true
                }
            }
        }

        return false
    }

}

export {ShapeMoveCommand}