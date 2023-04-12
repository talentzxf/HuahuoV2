import {ViewPort} from "./ViewPort";

class AxisSystem {
    originPoint: paper.Point = new paper.Point(0, 0)
    originCircle
    xAxisLine
    yAxisLine

    viewPort: ViewPort

    constructor(viewPort: ViewPort) {
        this.viewPort = viewPort
        // this.originCircle = new paper.Path.Circle(this.originPoint, 10)
        // this.originCircle.applyMatrix = false
        // this.originCircle.fillColor = new paper.Color("yellow")

        let xAxis = new paper.Point(0, 0)
        let yAxis = new paper.Point(0, 0)
        this.xAxisLine = new paper.Path.Line(this.originPoint, xAxis)
        this.yAxisLine = new paper.Path.Line(this.originPoint, yAxis)

        this.xAxisLine.strokeColor = new paper.Color("black")
        this.yAxisLine.strokeColor = new paper.Color("black")
    }

    setOriginPosition(x, y) {
        this.originPoint.x = x
        this.originPoint.y = y

        // this.originCircle.position = this.viewPort.viewPointToCanvasPoint(this.originPoint)
    }

    setXLength(xLength) {
        this.xAxisLine.segments[0].setPoint(this.viewPort.viewPointToCanvasPoint(this.originPoint))
        this.xAxisLine.segments[1].setPoint(this.viewPort.viewPointToCanvasPoint(new paper.Point(this.originPoint.x + xLength, this.originPoint.y)))
    }

    setYLength(yLength) {
        this.yAxisLine.segments[0].setPoint(this.viewPort.viewPointToCanvasPoint(this.originPoint))
        this.yAxisLine.segments[1].setPoint(this.viewPort.viewPointToCanvasPoint(new paper.Point(this.originPoint.x, this.originPoint.y + yLength)))
    }
}

export {AxisSystem}