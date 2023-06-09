import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {HHColorInput} from "./HHColorInput";
import {ColorStop} from "hhenginejs";
import {HHToast, getMethodsAndVariables} from "hhcommoncomponents";
import {createPenShape, selectedPenCapColor, unselectedPenCapColor} from "./Utils";
import {ArrayInsertCommand} from "../../RedoUndo/ArrayInsertCommand";
import {ArrayDeleteCommand} from "../../RedoUndo/ArrayDeleteCommand";
import {undoManager} from "../../RedoUndo/UndoManager";

const rectangleOffset = 10
const canvasWidth = 200
const canvasHeight = 20

const rectangleWidth = 150
const rectangleHeight = 20

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

    set fillColor(val: paper.Color) {
        this.colorStop.r = val.red
        this.colorStop.g = val.green
        this.colorStop.b = val.blue
        this.colorStop.a = val.alpha

        this.penBody.fillColor = val
    }

    get fillColor(): paper.Color {
        return this.penBody.fillColor
    }

    get selected() {
        return this._selected
    }

    constructor(colorStop?: ColorStop) {

        [this.paperGroup, this.penBody, this.penCap] = createPenShape()

        this.selected = false

        this.interceptPaperFunctions()

        if (colorStop)
            this.setColorStop(colorStop)

        this.paperGroup.data.meta = this
    }

    setColorStop(colorStop) {
        this.colorStop = colorStop
        this.penBody.fillColor = new paper.Color(colorStop.r, colorStop.g, colorStop.b, colorStop.a)
        this.paperGroup.position = new paper.Point(colorStop.value * rectangleWidth, rectangleHeight / 2)
    }

    remove() {
        this.paperGroup.remove()
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
    inserter: Function // This is actually inserter
    deleter: Function
    updater: Function

    canvas: HTMLCanvasElement
    rectangle: paper.Path

    pens: Array<Pen> = new Array<Pen>()
    projectId: number = -1

    hhColorInput: HHColorInput
    colorTitleSpan: HTMLSpanElement

    selectedPen: Pen = null

    kbEventAttached = false

    constructor(getter, inserter, updater, deleter) {
        super();

        this.updater = updater
        this.deleter = deleter
        this.getter = getter
        this.inserter = inserter

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
        this.rectangle.onClick = this.onRectangeClicked.bind(this)
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

        let kbEventListener = this.onKeyUp.bind(this)

        this.addEventListener("focusin", () => {
            if (this.kbEventAttached == false) {
                document.addEventListener("keyup", kbEventListener)
                this.kbEventAttached = true
            }

        })

        this.addEventListener("focusout", () => {
            document.removeEventListener("keyup", kbEventListener)
            this.kbEventAttached = false
        })

        this.tabIndex = 0;
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
                _this.colorTitleSpan.innerText = i18n.t("colorstops.pen.name", {index: pen.colorStop.identifier})

                _this.hhColorInput.value = pen.fillColor
                _this.selectedPen = pen

            } else {
                pen.selected = false
            }
        })
    }

    onKeyUp(evt: KeyboardEvent) {
        if (evt.code == "Delete") {

            if (this.pens.length <= 2) {
                HHToast.warn(i18n.t("toast.insufficientColorStop"))
            } else {
                let tobeDeletedPen = this.selectedPen

                if (tobeDeletedPen != null) {
                    let arrayDeleteCommand = new ArrayDeleteCommand(
                        () => {
                            let newlyInsertedPen = this.insertPenByValue(tobeDeletedPen.colorStop.value)
                            newlyInsertedPen.colorStop.r = tobeDeletedPen.colorStop.r
                            newlyInsertedPen.colorStop.g = tobeDeletedPen.colorStop.g
                            newlyInsertedPen.colorStop.b = tobeDeletedPen.colorStop.b
                            newlyInsertedPen.colorStop.a = tobeDeletedPen.colorStop.a

                            this.updater(newlyInsertedPen.colorStop)

                            this.refresh()

                            return newlyInsertedPen
                        },
                        (pen) => {
                            this.deletePen(pen)
                        }
                    )

                    this.deletePen(tobeDeletedPen)
                    undoManager.PushCommand(arrayDeleteCommand)


                }

                evt.stopPropagation()
            }
        }
    }

    insertPenByValue(value) {
        let insertedColorStopIdentifier = this.inserter(value) // Setter is actually inserter.
        this.refresh()

        let newlyAddedPen = null
        for (let pen of this.pens) {
            if (pen.colorStop.identifier == insertedColorStopIdentifier) {
                newlyAddedPen = pen
                this.selectPen(pen)
                break;
            }
        }

        return newlyAddedPen
    }

    deletePen(tobeDeletedPen) {
        this.pens = this.pens.filter((penInArray) => {
            return penInArray.colorStop.identifier != tobeDeletedPen.colorStop.identifier
        })

        // TODO: Do we really need to hide the color selector??
        // this.hhColorInput.hideColorSelector()

        this.deleter(tobeDeletedPen.colorStop)

        tobeDeletedPen.remove()

        this.selectPen(this.pens[0]) // Select the first pen to avoid confusion.

        this.refresh()
    }

    onRectangeClicked(evt: MouseEvent) {
        console.log(evt)

        let clickPoint = evt["point"]
        let value = clickPoint.x / rectangleWidth

        let arrayInsertCommand = new ArrayInsertCommand(
            () => {
                return this.insertPenByValue(value)
            },
            this.deletePen.bind(this)
        )

        undoManager.PushCommand(arrayInsertCommand)
        arrayInsertCommand.DoCommand()
    }

    onPenClicked(evt: MouseEvent) {
        console.log(evt)
        if (!evt.target["data"] || !evt.target["data"]["meta"]) {
            return
        }

        let pen = evt.target["data"]["meta"] as Pen
        this.selectPen(pen)
    }

    onPenMouseDrag(evt: paper.MouseEvent) {
        if (!evt.target["data"] || !evt.target["data"]["meta"]) {
            return
        }

        let pen = evt.target["data"]["meta"] as Pen

        this.selectPen(pen)

        let colorStop = pen.colorStop
        let deltaPortion = (evt["delta"].x / rectangleWidth)
        colorStop.value += deltaPortion

        // Clamp between 0-1
        colorStop.value = Math.min(colorStop.value, 1.0)
        colorStop.value = Math.max(0.0, colorStop.value)

        this.updater(colorStop)
        this.refresh()
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
                pen.paperGroup.onMouseDrag = this.onPenMouseDrag.bind(this)
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
