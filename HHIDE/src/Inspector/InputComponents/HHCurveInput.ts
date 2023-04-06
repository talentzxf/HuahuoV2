import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-curve-input"
})
class HHCurveInput extends HTMLElement{
    canvas: HTMLCanvasElement
    constructor() {
        super();

        this.canvas = document.createElement("canvas")

        this.appendChild(this.canvas)
    }
}

export {HHCurveInput}