import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {HHColorInput} from "./HHColorInput";
import {ColorStop} from "hhenginejs/dist/src/Components/ColorStop";
import {AbstractComponent} from "hhenginejs/dist/src/Components/AbstractComponent";

const rectangleOffset = 10
const canvasWidth = 200
const canvasHeight = 20

const rectangleWidth = 150
const rectangleHeight = 20
const penWidth = 10
const penHeight = 10
const penCapHeight = 10

const unselectedPenCapColor = new paper.Color("lightgray")
const selectedPenCapColor = new paper.Color("black")

const getMethodsAndVariables = (obj: any) => {
    let properties = new Set<any>()
    let currentObj = obj
    do {
        Object.getOwnPropertyNames(currentObj).map(item => properties.add(item))
    } while ((currentObj = Object.getPrototypeOf(currentObj)))
    return [...properties.keys()]
}

class Pen {
    colorStop: ColorStop

    paperGroup: paper.Group
    penCap: paper.Path
    penBody: paper.Path

    _selected: boolean

    set selected(val: boolean) {
        if (this._selected == val)
            return

        this._selected = val
        if (this._selected) {
            this.penCap.fillColor = selectedPenCapColor
        } else {
            this.penCap.fillColor = unselectedPenCapColor
        }
    }

    set fillColor(val: paper.Color){
        this.colorStop.r = val.red
        this.colorStop.g = val.green
        this.colorStop.b = val.blue
        this.colorStop.a = val.alpha

        this.penBody.fillColor = val
    }

    get fillColor(): paper.Color{
        return this.penBody.fillColor
    }

    get selected() {
        return this._selected
    }

    constructor(colorStop?: ColorStop) {

        let penGroup = new paper.Group()
        this.penBody = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(penWidth, penHeight))

        let penCapSegments = [new paper.Point(0, 0), new paper.Point(penWidth / 2, -penCapHeight), new paper.Point(penWidth, 0)]
        this.penBody.fillColor = new paper.Color("red")
        this.penBody.strokeColor = new paper.Color("black")
        this.penBody.strokeWidth = 3
        penGroup.addChild(this.penBody)

        this.penCap = new paper.Path(penCapSegments)
        this.penCap.closed = true
        this.penCap.fillColor = new paper.Color("lightgray")
        this.selected = false // refresh the pencap color.
        this.penCap.strokeColor = new paper.Color("black")
        this.penCap.strokeWidth = 3
        penGroup.addChild(this.penCap)

        this.paperGroup = penGroup

        this.interceptPaperFunctions()

        if (colorStop)
            this.setColorStop(colorStop)

        penGroup.data.meta = this
    }

    setColorStop(colorStop) {
        this.colorStop = colorStop
        this.penBody.fillColor = new paper.Color(colorStop.r, colorStop.g, colorStop.b, colorStop.a)
        this.paperGroup.position = new paper.Point(colorStop.value * rectangleWidth, rectangleHeight / 2)
    }

    interceptPaperFunctions() {
        // Proxy all paperGroup functions/methods.
        let _this = this
        getMethodsAndVariables(this.paperGroup).forEach(key => {
            if (key == "constructor")
                return
            const originalProp = this.paperGroup[key]

            if ("function" === typeof originalProp) {
                _this[key] = (...args) => {
                    return Reflect.apply(originalProp, _this.paperGroup, args)
                }
            }
            // else {
            //     Object.defineProperty(_this, key, {
            //         get: function () {
            //             return _this.paperGroup[key]
            //         },
            //         set: function (val) {
            //             _this.paperGroup[key] = val
            //         }
            //     })
            // }
        })
    }
}

@CustomElement({
    selector: "hh-color-stop-array-input"
})
class HHColorStopArrayInput extends HTMLElement implements RefreshableComponent {
    getter: Function
    setter: Function
    deleter: Function
    updater: Function

    canvas: HTMLCanvasElement
    rectangle: paper.Path

    pens: Array<paper.Group> = new Array<paper.Group>()
    projectId: number = -1

    hhColorInput: HHColorInput
    colorTitleSpan: HTMLSpanElement

    selectedPen: Pen = null

    constructor(getter, setter, updater, deleter) {
        super();

        this.updater = updater
        this.deleter = deleter
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

        let colorTitle = document.createElement("div")
        this.colorTitleSpan = document.createElement("span")

        this.colorTitleSpan.innerText = "unselected pen"
        colorTitle.appendChild(this.colorTitleSpan)

        let _this = this
        this.hhColorInput = new HHColorInput(this.colorStopColorChanged.bind(this), () => {
            if (_this.selectedPen && _this.selectedPen.penBody)
                return _this.selectedPen.penBody.fillColor
            return unselectedPenCapColor
        }, colorTitle)

        this.appendChild(colorTitle)
        this.appendChild(this.hhColorInput)

        // Select the first pen
        this.selectPen(this.pens[0])
    }

    colorStopColorChanged(val: paper.Color) {
        if (this.selectedPen && this.selectedPen.penBody) {
            this.selectedPen.fillColor = val
            this.updater(this.selectedPen.colorStop)

            this.refresh()
        }
    }

    selectPen(selectedPen: Pen) {
        let _this = this
        this.pens.forEach((pen) => {
            if (pen == selectedPen) {
                pen.selected = true
                _this.colorTitleSpan.innerText = i18n.t("colorstops.pen.name", {index: pen.colorStop.index})

                _this.hhColorInput.value = pen.fillColor
                _this.selectedPen = pen

            } else {
                pen.selected = false
            }
        })
    }

    onPenClicked(evt:MouseEvent){
        console.log(evt)
        if(!evt.target["data"] || !evt.target["data"]["meta"]){
            return
        }

        let pen = evt.target["data"]["meta"] as Pen
        this.selectPen(pen)
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
                pen = new Pen(colorStop)

                pen.paperGroup.onClick = this.onPenClicked.bind(this)
                this.pens.push(pen)
            }

            pen.setColorStop(colorStop)

            pen.bringToFront()
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
