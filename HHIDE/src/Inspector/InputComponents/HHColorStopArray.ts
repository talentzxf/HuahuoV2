import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"

const canvasWidth = 150
const canvasHeight = 20

@CustomElement({
    selector: "hh-color-stop-array-input"
})
class HHColorStopArrayInput extends HTMLElement implements RefreshableComponent{
    getter: Function
    setter: Function

    canvas: HTMLCanvasElement
    constructor(getter, setter) {
        super();

        this.getter = getter
        this.setter = setter

        this.canvas = document.createElement("canvas")
        this.canvas.style.width = canvasWidth + "px"
        this.canvas.style.height = canvasHeight + "px"
        this.canvas.width = canvasWidth
        this.canvas.height = canvasHeight

        paper.setup(this.canvas)

        let rectangle = new paper.Path.Rectangle(new paper.Point(0,0), new paper.Point(canvasWidth, canvasHeight))
        rectangle.fillColor = new paper.Color({
            gradient:{
                stops:[['yellow', 0.00], ["yellow", 1.0]]
            },
            origin: rectangle.bounds.leftCenter,
            destination: rectangle.bounds.rightCenter
        })

        this.appendChild(this.canvas)
    }

    refresh() {
    }

}

export {HHColorStopArrayInput}
