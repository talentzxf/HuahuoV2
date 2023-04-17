class ViewPort {
    canvasWidth
    canvasHeight

    viewWidth
    viewHeight

    viewXMin
    viewYMin
    viewXMax
    viewYMax
    leftDown

    get viewXSpan() {
        return this.viewXMax - this.viewXMin
    }

    get viewYSpan() {
        return this.viewYMax - this.viewYMin
    }

    getXOffsetForFrame(frameId: number) {
        let xScale = this.viewWidth / this.viewXSpan
        return this.leftDown[0] + (frameId - this.viewXMin) * xScale
    }

    getFrameIdFromXOffset(xoffset: number){
        let xScale = this.viewWidth / this.viewXSpan

        return Math.floor((xoffset - this.leftDown[0])/xScale + this.viewXMin)
    }

    viewPointToCanvasPoint(p) {
        let [x, y] = this.viewToCanvas(p.x, p.y)
        return new paper.Point(x, y)
    }

    getValueFromYOffset(yoffset: number){
        let yScale = this.viewHeight / this.viewYSpan
        return this.viewYMin + (this.leftDown[1] - yoffset)/yScale
    }

    viewToCanvas(x, y) {
        let xScale = this.viewWidth / this.viewXSpan
        let yScale = this.viewHeight / this.viewYSpan

        let canvasX = this.leftDown[0] + (x - this.viewXMin) * xScale
        let canvasY = this.leftDown[1] - (y - this.viewYMin) * yScale

        return [canvasX, canvasY]
    }

    canvasPointToViewPoint(canvasX, canvasY){
        let xScale = this.viewWidth / this.viewXSpan
        let yScale = this.viewHeight / this.viewYSpan

        let viewPointX = this.viewXMin + (canvasX - this.leftDown[0])/xScale
        let viewPointY = this.viewYMin + (this.leftDown[1] - canvasY)/yScale

        return [viewPointX, viewPointY]
    }
}

export {ViewPort}