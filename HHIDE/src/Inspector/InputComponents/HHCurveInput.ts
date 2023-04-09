import {CustomElement} from "hhcommoncomponents";

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

    viewToCanvas(x, y) {
        let xScale = this.viewWidth / this.viewXSpan
        let yScale = this.viewHeight / this.viewYSpan

        let canvasX = this.leftDown[0] + (x - this.viewXMin) * xScale
        let canvasY = this.leftDown[1] - (y - this.viewYMin) * yScale

        return [canvasX, canvasY]
    }
}

@CustomElement({
    selector: "hh-curve-input"
})
class HHCurveInput extends HTMLElement {
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D

    keyFrameCurveGetter: Function
    viewPort: ViewPort = new ViewPort()

    constructor(keyFrameCurveGetter) {
        super();

        this.keyFrameCurveGetter = keyFrameCurveGetter

        this.canvas = document.createElement("canvas")
        this.ctx = this.canvas.getContext("2d")

        this.appendChild(this.canvas)
    }

    connectedCallback() {
        this.refresh()
    }

    refresh() {
        let curve = this.keyFrameCurveGetter()
        if (curve == null)
            return

        let minValue = Number.MAX_VALUE
        let maxValue = -Number.MAX_VALUE

        let minFrameId = Number.MAX_VALUE
        let maxFrameId = -Number.MAX_VALUE

        let points = []
        let totalPoints = curve.GetTotalPoints()
        for (let pointIdx = 0; pointIdx < totalPoints; pointIdx++) {
            let curvePoint = curve.GetKeyFrameCurvePoint(pointIdx)
            let value = curvePoint.GetValue()
            let frameId = curvePoint.GetFrameId() + 1 // In Cpp side, frameId starts from 0. But when shown, frameId starts from 1.

            minValue = Math.min(minValue, value)
            maxValue = Math.max(maxValue, value)
            minFrameId = Math.min(minFrameId, frameId)
            maxFrameId = Math.max(maxFrameId, frameId)

            points.push(curvePoint)
        }

        // Clear background
        this.ctx.fillStyle = "lightgray"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        // Add some offset to avoid 0/0
        if (minFrameId == maxFrameId) {
            maxFrameId = minFrameId + 1
        }

        if (minValue == maxValue) {
            maxValue = minValue + 1
        }

        // Draw the coordinate
        this.viewPort.canvasWidth = this.canvas.width
        this.viewPort.canvasHeight = this.canvas.height
        this.viewPort.viewWidth = 0.8 * this.canvas.width
        this.viewPort.viewHeight = 0.8 * this.canvas.height
        this.viewPort.viewXMin = minFrameId
        this.viewPort.viewXMax = maxFrameId
        this.viewPort.viewYMin = minValue
        this.viewPort.viewYMax = maxValue
        this.viewPort.leftDown = [0.05 * this.canvas.width, 0.9 * this.canvas.height]

        let canvasOrigin = this.viewPort.viewToCanvas(minFrameId, minValue)

        let xMax = this.viewPort.viewToCanvas(maxFrameId, minValue)
        let yMax = this.viewPort.viewToCanvas(minFrameId, maxValue)

        this.ctx.beginPath()
        this.ctx.moveTo(canvasOrigin[0], canvasOrigin[1])
        this.ctx.lineTo(xMax[0], xMax[1])
        this.ctx.moveTo(canvasOrigin[0], canvasOrigin[1])
        this.ctx.lineTo(yMax[0], yMax[1])

        this.ctx.strokeStyle = "green"
        this.ctx.stroke()

        this.ctx.font = "10px serif"
        this.ctx.fillStyle = "black"

        // Draw lines
        for (let point of points) {
            let frameId = point.GetFrameId() + 1
            let value = point.GetValue()

            this.ctx.beginPath()
            let canvasPoint = this.viewPort.viewToCanvas(frameId, value)
            this.ctx.arc(canvasPoint[0], canvasPoint[1], 3, 0, 2 * Math.PI, true)
            this.ctx.fillStyle = "blue"
            this.ctx.fill()

            // Draw X-Axis text.
            let position = this.viewPort.viewToCanvas(frameId, minValue)
            let string = String(frameId)
            let textRect = this.ctx.measureText(string)

            let actualHeight = textRect.actualBoundingBoxAscent + textRect.actualBoundingBoxDescent;
            this.ctx.fillText(string, position[0] - textRect.width/2.0, position[1] + actualHeight + 5)

            // Draw Y-Axis text.
            position = this.viewPort.viewToCanvas(minFrameId, value)
            string = String(value)
            textRect = this.ctx.measureText(string)
            actualHeight = textRect.actualBoundingBoxAscent + textRect.actualBoundingBoxDescent;
            this.ctx.fillText(string, position[0] - textRect.width - 5, position[1] + actualHeight/2.0)
        }
    }

}

export {HHCurveInput}