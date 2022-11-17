import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"

const rectangleOffset = 10
const canvasWidth = 200
const canvasHeight = 20

const rectangleWidth = 150
const rectangleHeight = 20
const penWidth = 10
const penHeight = 10
const penCapHeight = 10

const getMethodsAndVariables = (obj: any) => {
    let properties = new Set<any>()
    let currentObj = obj
    do {
        Object.getOwnPropertyNames(currentObj).map(item => properties.add(item))
    } while ((currentObj = Object.getPrototypeOf(currentObj)))
    return [...properties.keys()]
}

class Pen {
    paperGroup: paper.Group

    constructor() {
        let penGroup = new paper.Group()
        let penBody = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(penWidth, penHeight))

        let penCapSegments = [new paper.Point(0,0), new paper.Point(penWidth/2, -penCapHeight), new paper.Point(penWidth, 0)]
        penBody.fillColor = new paper.Color("red")
        penBody.strokeColor = new paper.Color("blue")
        penGroup.addChild(penBody)

        let penCap = new paper.Path(penCapSegments)
        penCap.fillColor = new paper.Color("red")
        penCap.strokeColor = new paper.Color("blue")
        penGroup.addChild(penCap)

        this.paperGroup = penGroup

        // Proxy all paperGroup functions/methods.
        let _this = this
        getMethodsAndVariables(this.paperGroup).forEach(key => {
            if(key == "constructor")
                return
            const originalProp = this.paperGroup[key]

            if("function" === typeof originalProp){
                _this[key] = (...args) => {
                    return Reflect.apply(originalProp, _this.paperGroup, args)
                }
            }else{
                Object.defineProperty(_this, key, {
                    get: function(){
                        return _this.paperGroup[key]
                    },
                    set: function(val){
                        _this.paperGroup[key] = val
                    }
                })
            }
        })
    }
}

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
        paper.project.view.translate(new paper.Point(rectangleOffset, 0))

        this.rectangle = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(rectangleWidth, rectangleHeight))
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
                pen = new Pen()
                this.pens.push(pen)
            }

            pen.bringToFront()
            pen.position = new paper.Point( colorStop.value * rectangleWidth, rectangleHeight/2)
            pen.fillColor = new paper.Color( colorStop.r, colorStop.g, colorStop.b, colorStop.a)
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
