import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {ColorStop} from "hhenginejs/dist/src/Components/ColorStop";

const canvasWidth = 150
const canvasHeight = 20

@CustomElement({
    selector: "hh-color-stop-array-input"
})
class HHColorStopArrayInput extends HTMLElement implements RefreshableComponent{
    getter: Function
    setter: Function

    canvas: HTMLCanvasElement
    rectangle: paper.Path

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

        this.rectangle = new paper.Path.Rectangle(new paper.Point(0,0), new paper.Point(canvasWidth, canvasHeight))
        this.appendChild(this.canvas)

        this.refresh()
    }

    refresh() {
        let colorStopArray = this.getter()

        let stops = []
        for(let colorStop of colorStopArray){
            let color = new paper.Color(colorStop.r, colorStop.g, colorStop.b, colorStop.a)

            stops.push([color, colorStop.value])
        }

        this.rectangle.fillColor = new paper.Color({
            gradient:{
                stops:stops
            },
            origin: this.rectangle.bounds.leftCenter,
            destination: this.rectangle.bounds.rightCenter
        })

    }

}

export {HHColorStopArrayInput}
