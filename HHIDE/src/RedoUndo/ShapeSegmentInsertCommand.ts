import {UndoableCommand} from "./UndoManager";

class ShapeSegmentInsertCommand extends UndoableCommand{
    shape
    position

    segment = null
    constructor(shape, position) {
        super();

        this.shape = shape
        this.position = position
    }

    GetType(): string {
        return "ShapeSegmentInsertCommand";
    }

    _DoCommand() {
        let localPos = this.shape.globalToLocal(this.position)
        let nearestPoint = this.shape.getNearestPoint(localPos)
        let offset = this.shape.getOffsetOf(nearestPoint)

        let newSegment = this.shape.divideAt(offset)
        this.shape.insertSegment(localPos)

        this.segment = newSegment

        return newSegment
    }

    _UnDoCommand() {
        if(this.segment){
            // Delete the segment
            this.shape.removeSegment(this.segment)
            this.segment = null
        }
    }

}

export {ShapeSegmentInsertCommand}