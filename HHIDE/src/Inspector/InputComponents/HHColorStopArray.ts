import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {ColorStop} from "hhenginejs/dist/src/Components/ColorStop";

const canvasWidth = 150
const canvasHeight = 20

@CustomElement({
    selector: "hh-color-stop-array-input"
})
class HHColorStopArrayInput extends HTMLElement implements RefreshableComponent {
    getter: Function
    setter: Function

    canvas: HTMLCanvasElement
    rectangle: paper.Path

    pens: Array<paper.Group> = new Array<paper.Group>()
    projectId: number = -1

    constructor(getter, setter) {
        super();

        this.getter = getter
        this.setter = setter

        this.canvas = document.createElement("canvas")
        this.canvas.style.width = canvasWidth + "px"
        this.canvas.style.height = canvasHeight + "px"
        this.canvas.width = canvasWidth
        this.canvas.height = canvasHeight

        let previousPaperProject = paper.project
        paper.setup(this.canvas)
        this.projectId = paper.project.index

        this.rectangle = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(canvasWidth, canvasHeight))
        this.refresh()

        this.appendChild(this.canvas)

        previousPaperProject.activate()
    }

    refresh() {
        let oldProjectId = -1
        if (paper.project.index != this.projectId) {
            oldProjectId = paper.project.index

            paper.projects[this.projectId].activate()
        }

        let colorStopArray = this.getter()

        let penIndex = 0

        let stops = []
        for (let colorStop of colorStopArray) {
            let color = new paper.Color(colorStop.r, colorStop.g, colorStop.b, colorStop.a)

            stops.push([color, colorStop.value])

            let pen = null
            // Draw pens.
            if (penIndex < this.pens.length) { // Reuse previously created pens
                pen = this.pens[penIndex]
            } else { // Create new pen.
                // let penGroup = new paper.Group()
                let penBody = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(10, 10))
                penBody.fillColor = new paper.Color("red")
                penBody.strokeColor = new paper.Color("blue")
                // penGroup.addChild(penBody)

                pen = penBody

                this.pens.push(penBody)
            }

            pen.bringToFront()
            // let newPen = new paper.Group()
            // let penBody = new paper.Path.Rectangle(new paper.Point(0,0), new paper.Point(10, canvasHeight/2))
            // penBody.fillColor = "red"
            //
            // newPen.addChild(penBody)

            penIndex++
        }

        this.rectangle.fillColor = new paper.Color({
            gradient: {
                stops: stops
            },
            origin: this.rectangle.bounds.leftCenter,
            destination: this.rectangle.bounds.rightCenter
        })

        this.rectangle.sendToBack()

        if (oldProjectId != -1) {
            paper.projects[oldProjectId].activate()
        }
    }

}

export {HHColorStopArrayInput}
