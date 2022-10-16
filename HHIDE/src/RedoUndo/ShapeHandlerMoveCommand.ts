import {MergableCommand} from "./UndoManager";

let commandName = "ShapeHandlerMoveCommand"

class ShapeHandlerMoveCommand extends MergableCommand {
    shape
    segment
    handleName
    prevPosition
    targetPosition

    constructor(shape, segment, handleName, prevPosition, targetPosition) {
        super();

        this.shape = shape
        this.segment = segment
        this.handleName = handleName
        this.prevPosition = prevPosition
        this.targetPosition = targetPosition
    }

    GetType(): string {
        return commandName;
    }

    MergeCommand(anotherCommand: MergableCommand): boolean {
        if (anotherCommand.GetType() == commandName) {
            let moveCommand = anotherCommand as ShapeHandlerMoveCommand
            if (moveCommand) {
                if (this.stackFrameEqual(anotherCommand)) {
                    // This is the same shape, in the same layer, in the same frameId, can merge the move command
                    if (moveCommand.shape == this.shape
                        && this.segment == moveCommand.segment
                        && this.prevPosition == moveCommand.prevPosition) {
                        this.targetPosition = moveCommand.targetPosition
                        return true
                    }
                }
            }
        }
        return false;
    }

    _DoCommand() {
        this.segment[this.handleName] = this.targetPosition
        this.shape.store()
    }

    _UnDoCommand() {
        this.segment[this.handleName] = this.prevPosition
        this.shape.store()
    }

}

export {ShapeHandlerMoveCommand}