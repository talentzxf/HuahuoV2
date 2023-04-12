
let eps = 0.001
class MovableCurve {
    private paperShape: paper.Path

    constructor(paperObj: paper.Path) {
        this.paperShape = paperObj
    }

    isMovable() {
        return true
    }

    isSegmentSeletable() {
        return true
    }

    globalToLocal(point) {
        return this.paperShape.globalToLocal(point)
    }

    setSegmentProperty(idx, property, value) {
        let segment = this.paperShape.segments[idx]
        let currentValue = this.paperShape.segments[idx][property]

        if (Math.abs(currentValue - value) < eps)
            return
        this.paperShape.segments[idx][property] = value
    }

    getOffsetOf(point: paper.Point) {
        return this.paperShape.getOffsetOf(point)
    }

    divideAt(offset: number) {
        return this.paperShape.divideAt(offset)
    }

    insertSegment(localPos: paper.Point) {
    }

    getNearestPoint(point: paper.Point) {
        return this.paperShape.getNearestPoint(point)
    }

    // Save the information for Ctrl-Z
    getFrameIdSegmentsBuffer() {

    }

    // This will be executed during Ctrl-Z
    restoreFrameSegmentsBuffer(previouslySavedSegmentsBuffer) {

    }

    // TODO: Store the result
    store() {

    }

    update() {

    }
}

export {MovableCurve}