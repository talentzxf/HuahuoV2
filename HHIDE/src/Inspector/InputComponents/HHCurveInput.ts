import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-curve-input"
})
class HHCurveInput extends HTMLElement {
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D

    keyFrameCurveGetter: Function

    constructor(keyFrameCurveGetter) {
        super();

        this.keyFrameCurveGetter = keyFrameCurveGetter

        this.canvas = document.createElement("canvas")
        this.ctx = this.canvas.getContext("2d")
        this.ctx.globalCompositeOperation = "destination-over"
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
        let maxValue = Number.MIN_VALUE

        let minFrameId = Number.MAX_VALUE
        let maxFrameId = Number.MIN_VALUE

        let totalPoints = curve.GetTotalPoints()
        for (let pointIdx = 0; pointIdx < totalPoints; pointIdx++) {
            let curvePoint = curve.GetKeyFrameCurvePoint[pointIdx]
            let value = curvePoint.GetValue()
            let frameId = curvePoint.GetFrameId()

            minValue = Math.min(minValue, value)
            maxValue = Math.max(maxValue, value)
            minFrameId = Math.min(minFrameId, frameId)
            maxFrameId = Math.max(maxFrameId, frameId)
        }

        // Draw the coordinate
        let canvasWidth = this.canvas.width
        let canvasHeight = this.canvas.height

        let origin = [0.1 * canvasWidth, 0.1 * canvasHeight]
        let x = [0.9 * canvasWidth, 0.1 * canvasHeight]
        let y = [0.1 * canvasWidth, 0.9 * canvasHeight]
        this.ctx.beginPath()
        this.ctx.moveTo(origin[0], origin[1])
        this.ctx.lineTo(x[0], x[1])
        this.ctx.moveTo(origin[0], origin[1])
        this.ctx.lineTo(y[0], y[1])
        this.ctx.stroke()
    }

}

export {HHCurveInput}