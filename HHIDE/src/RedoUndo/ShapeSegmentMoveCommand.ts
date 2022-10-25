import {MergableCommand} from "./UndoManager";

let cmdType = "ShapeSegmentMoveCommand"
class ShapeSegmentMoveCommand extends MergableCommand{
    GetType(): string {
        return cmdType;
    }

    shape
    // Only store the segment index. Can't store the segment itself.
    // As the segment might change after the shape is create/recreated
    segmentIdx
    prevPosition
    targetPosition

    constructor(shape, segmentIdx, prevPosition, targetPosition) {
        super();

        this.shape = shape
        this.segmentIdx = segmentIdx
        this.prevPosition = prevPosition
        this.targetPosition = targetPosition
    }

    MergeCommand(anotherCommand: MergableCommand): boolean {
        if (anotherCommand.GetType() == cmdType) {
            let moveCommand = anotherCommand as ShapeSegmentMoveCommand
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
        this.shape.setSegmentProperty(this.segmentIdx, "point", this.targetPosition);
        // After morph, the position of the shape might be shifted, so we need to store the new position in the Cpp side.
        this.shape.store()
    }

    _UnDoCommand() {
        this.shape.setSegmentProperty(this.segmentIdx, "point", this.prevPosition);
        // After morph, the position of the shape might be shifted, so we need to store the new position in the Cpp side.
        this.shape.store()
    }

}

export {ShapeSegmentMoveCommand}