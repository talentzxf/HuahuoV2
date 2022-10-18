import {UndoableCommand} from "./UndoManager";

class ShapeSegmentInsertCommand extends UndoableCommand{
    shape
    position

    segment = null
    frameSegmentsBuffer = null
    constructor(shape, position) {
        super();

        this.shape = shape
        this.position = position
    }

    GetType(): string {
        return "ShapeSegmentInsertCommand";
    }

    _DoCommand() {
        // Store current segments buffer
        this.frameSegmentsBuffer = this.shape.getFrameIdSegmentsBuffer()

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
            this.shape.restoreFrameSegmentsBuffer(this.frameSegmentsBuffer)
        }
    }

}

export {ShapeSegmentInsertCommand}