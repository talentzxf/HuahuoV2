import {CustomElement} from "hhcommoncomponents";
class ViewPort{
    canvasWidth
    canvasHeight

    viewWidth
    viewHeight

    viewXMin
    viewYMin
    viewXMax
    viewYMax
    origin

    get viewXSpan(){
        if(this.viewXMax == this.viewXMin)
            return 1
        return this.viewXMax - this.viewXMin
    }

    get viewYSpan(){
        if(this.viewYMin == this.viewYMax)
            return 1;
        return this.viewYMax - this.viewYMin
    }

    viewToCanvas(x, y){
        let xScale = this.viewXSpan/this.viewWidth
        let yScale = this.viewYSpan/this.viewHeight

        let canvasX = this.origin[0] + x * xScale
        let canvasY = this.canvasHeight - (this.origin[1] + y * yScale)

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
        this.ctx.fillStyle = "lightgray"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        this.appendChild(this.canvas)
    }

    connectedCallback() {
        this.refresh()
    }

    refresh() {
        let curve = this.keyFrameCurveGetter()
        if(curve == null)
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

        // Draw the coordinate
        this.viewPort.canvasWidth = this.canvas.width
        this.viewPort.canvasHeight = this.canvas.height
        this.viewPort.viewWidth = 0.9 * this.canvas.width
        this.viewPort.viewHeight = 0.9 * this.canvas.height
        this.viewPort.viewXMin = minFrameId
        this.viewPort.viewXMax = maxFrameId
        this.viewPort.viewYMin = minValue
        this.viewPort.viewYMax = maxValue
        this.viewPort.origin = [0.1 * this.canvas.width, 0.1 * this.canvas.height]

        let canvasOrigin = this.viewPort.viewToCanvas(0, 0)

        let xMax = this.viewPort.viewToCanvas(maxFrameId,0)
        let yMax = this.viewPort.viewToCanvas(0, maxValue)

        this.ctx.beginPath()
        this.ctx.moveTo(canvasOrigin[0], canvasOrigin[1])
        this.ctx.lineTo(xMax[0], xMax[1])
        this.ctx.moveTo(canvasOrigin[0], canvasOrigin[1])
        this.ctx.lineTo(yMax[0], yMax[1])

        this.ctx.strokeStyle = "green"
        this.ctx.stroke()

        // Draw lines
        for(let point of points){

        }
    }

}

export {HHCurveInput}