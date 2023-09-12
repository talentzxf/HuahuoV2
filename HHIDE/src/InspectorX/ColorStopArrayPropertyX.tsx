import * as React from "react"
import {PropertyEntry, PropertyProps, registerPropertyChangeListener} from "./BasePropertyX";
import {createPenShape, selectedPenCapColor, unselectedPenCapColor} from "../Inspector/InputComponents/Utils";
import {ColorStop, paper} from "hhenginejs";
import {getMethodsAndVariables} from "hhcommoncomponents";
import {PropertyChangeListener} from "./PropertyChangeListener";
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {ColorPropertyX} from "./ColorPropertyX";
import {ArrayDeleteCommand} from "../RedoUndo/ArrayDeleteCommand";
import {HHToast} from "hhcommoncomponents";

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

    hide() {
        this.paperGroup.visible = false
    }

    show() {
        this.paperGroup.visible = true
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

let colorStopArrayPtrProjectIdMap = new Map()

type ColorStopArrayPropertyState = {
    colorTitle: string
}

class ColorStopArrayPropertyX extends React.Component<PropertyProps, ColorStopArrayPropertyState> implements PropertyChangeListener {
    state: ColorStopArrayPropertyState = {
        colorTitle: "unselected pen"
    }

    canvasRef
    rectangle: paper.Path.Rectangle

    pens: Array<Pen> = new Array<Pen>()

    selectedPen: Pen = null

    constructor(props) {
        super(props);
        this.canvasRef = React.createRef()
    }

    onRectangleClicked() {

    }

    componentDidMount() {
        let property = this.props.property
        let colorStopArray = property.getter()
        let colorStopArrayPtr = colorStopArray.ptr

        let previousPaperProject = paper.project
        paper.setup(this.canvasRef.current)
        let projectId = paper.project.index
        colorStopArrayPtrProjectIdMap.set(colorStopArrayPtr, projectId)
        paper.project.view.translate(new paper.Point(rectangleOffset, 0))

        this.rectangle = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(rectangleWidth, rectangleHeight))
        this.rectangle.onClick = this.onRectangleClicked.bind(this)
        this.refresh()
        previousPaperProject.activate()

        // Select the first pen
        if (this.pens.length > 0)
            this.selectPen(this.pens[0])
    }

    selectPen(selectedPen: Pen) {
        let _this = this
        this.pens.forEach((pen) => {
            if (pen == selectedPen) {
                pen.selected = true

                _this.state.colorTitle = i18n.t("colorstops.pen.name", {index: pen.colorStop.identifier})
                _this.selectedPen = pen

            } else {
                pen.selected = false
            }
        })

        this.setState(this.state)
    }

    onPenClicked(evt: MouseEvent) {
        console.log(evt)
        if (!evt.target["data"] || !evt.target["data"]["meta"]) {
            return
        }

        let pen = evt.target["data"]["meta"] as Pen
        this.selectPen(pen)
    }

    colorStopValueSetterFunctionMap: Map<ColorStop, Function> = new Map()

    colorStopValueSetter(colorStop) {

        if (!this.colorStopValueSetterFunctionMap.has(colorStop.identifier)) {
            this.colorStopValueSetterFunctionMap.set(colorStop.identifier, (value) => {
                colorStop.value = value
                this.props.property.updater(colorStop)
                this.refresh()
            })
        }

        return this.colorStopValueSetterFunctionMap.get(colorStop.identifier)
    }

    onPenMouseDrag(evt: paper.MouseEvent) {
        if (!evt.target["data"] || !evt.target["data"]["meta"]) {
            return
        }

        let pen = evt.target["data"]["meta"] as Pen

        this.selectPen(pen)

        let colorStop = pen.colorStop
        let deltaPortion = (evt["delta"].x / rectangleWidth)

        let oldValue = colorStop.value


        let newValue = oldValue + deltaPortion

        // Clamp between 0-1
        newValue = Math.min(newValue, 1.0)
        newValue = Math.max(0.0, newValue)

        let setFieldValueCommand = new SetFieldValueCommand(this.colorStopValueSetter(colorStop), oldValue, newValue)

        setFieldValueCommand.DoCommand()
        undoManager.PushCommand(setFieldValueCommand)
    }

    onValueChanged(val: any): void {
    }

    refresh() {
        let colorStopArray = this.props.property.getter()
        let projectId = colorStopArrayPtrProjectIdMap.get(colorStopArray.ptr)
        let oldProjectId = -1
        if (paper.project.index != projectId) {
            oldProjectId = paper.project.index

            paper.projects[projectId].activate()
        }

        let penIndex = 0

        let stops = []
        for (let colorStop of colorStopArray) {
            let color = new paper.Color(colorStop.r, colorStop.g, colorStop.b, colorStop.a)

            stops.push([color, colorStop.value])

            let pen = null
            // Draw pens.
            if (penIndex < this.pens.length) { // Reuse previously created pens
                pen = this.pens[penIndex]
                pen.show()
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

        for (let penIdx = penIndex; penIdx < this.pens.length; penIdx++) {
            this.pens[penIdx].hide()
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

    insertPenByValue(value) {
        let insertedColorStopIdentifier = this.props.property.inserter(value) // Setter is actually inserter.
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

                            this.props.property.updater(newlyInsertedPen.colorStop)

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

    deletePen(tobeDeletedPen) {
        this.pens = this.pens.filter((penInArray) => {
            return penInArray.colorStop.identifier != tobeDeletedPen.colorStop.identifier
        })

        // TODO: Do we really need to hide the color selector??
        // this.hhColorInput.hideColorSelector()

        this.props.property.deleter(tobeDeletedPen.colorStop)

        tobeDeletedPen.remove()

        this.selectPen(this.pens[0]) // Select the first pen to avoid confusion.

        this.refresh()
    }

    colorStopColorChanged(val: paper.Color) {
        if (this.selectedPen && this.selectedPen.penBody) {
            this.selectedPen.fillColor = val
            this.props.property.updater(this.selectedPen.colorStop)

            this.refresh()
        }
    }

    render() {
        registerPropertyChangeListener(this, this.props.property)

        return (
            <PropertyEntry className="col-span-2" onKeyUp={this.onKeyUp.bind(this)} property={this.props.property}>
                <label className="mx-1 px-1">{this.state.colorTitle}</label>
                <ColorPropertyX property={{
                    getter: () => {
                        if (this.selectedPen && this.selectedPen.penBody)
                            return this.selectedPen.penBody.fillColor
                        return unselectedPenCapColor
                    },
                    setter: this.colorStopColorChanged.bind(this)
                }}></ColorPropertyX>
                <canvas ref={this.canvasRef} width={canvasWidth} height={canvasHeight} style={{
                    width: canvasWidth + "px",
                    height: canvasHeight + "px"
                }}></canvas>
            </PropertyEntry>)
    }
}

export {ColorStopArrayPropertyX}