import {MergableCommand} from "./UndoManager";

let commandName = "ShapeSegmentMoveCommand"
class ShapeSegmentMoveCommand extends MergableCommand{
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

export {ShapeSegmentMoveCommand}