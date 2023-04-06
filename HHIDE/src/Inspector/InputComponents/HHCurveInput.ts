import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-curve-input"
})
class HHCurveInput extends HTMLElement {
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D

    constructor() {
        super();

        this.canvas = document.createElement("canvas")
        this.ctx = this.canvas.getContext("2d")
        this.ctx.globalCompositeOperation = "destination-over"
        this.ctx.fillStyle = "lightgray"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        this.appendChild(this.canvas)
    }
}

export {HHCurveInput}