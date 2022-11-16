import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"

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

        paper.setup(this.canvas)

        let rectangle = new paper.Path.Rectangle(new paper.Point(0,0), new paper.Point(100,100))
        rectangle.fillColor = new paper.Color("black")

        this.appendChild(this.canvas)
    }

    refresh() {
    }

}

export {HHColorStopArrayInput}
