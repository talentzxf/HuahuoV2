import {MergableCommand} from "./UndoManager";

let commandName = "ShapeHandlerMoveCommand"

class ShapeHandlerMoveCommand extends MergableCommand {
    shape
    segmentIdx
    handleName
    prevPosition
    targetPosition

    constructor(shape, segmentIdx, handleName, prevPosition, targetPosition) {
        super();

        this.shape = shape
        this.segmentIdx = segmentIdx
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
                        && this.segmentIdx == moveCommand.segmentIdx
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
        this.shape.setSegmentProperty(this.segmentIdx, this.handleName, this.targetPosition);
        this.shape.store()
        this.shape.update(true)
    }

    _UnDoCommand() {
        this.shape.setSegmentProperty(this.segmentIdx, this.handleName, this.prevPosition);
        this.shape.store()
        this.shape.update(true)
    }

}

export {ShapeHandlerMoveCommand}