import * as React from "react"
import {paper} from "hhenginejs"
import {ContextMenu} from "hhcommoncomponents";
import {createPenShape, selectedPenCapColor, unselectedPenCapColor} from "../Inspector/InputComponents/Utils";
import {HHToast} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";

const penOffset = 10
const textMargin = 5
const spanRectHeight = 5

function triggerFocus(element) {
    var eventType = "focusin",
        bubbles = null,
        event;

    if ("createEvent" in document) {
        event = document.createEvent("Event");
        event.initEvent(eventType, bubbles, true);
    } else if ("Event" in window) {
        event = new Event(eventType, {bubbles: bubbles, cancelable: true});
    }

    element.focus();
    element.dispatchEvent(event);
}

class TimelinePointer {
    paperGroup: paper.Group
    penCap: paper.Path
    penBody: paper.Path
    penText: paper.PointText
    _selected: boolean = false

    frameId: number = -1
    cellWidth: number = -1
    minFrameId: number = -1

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

    constructor() {
        [this.paperGroup, this.penBody, this.penCap] = createPenShape()
        this.paperGroup.data.meta = this

        this.paperGroup.rotation = 180

        let penBounds = this.penBody.bounds
        this.penText = new paper.PointText(penBounds.topLeft.add(penBounds.topRight).divide(2.0).add(new paper.Point(0, -textMargin)))
        this.penText.justification = "center"
        this.penText.content = "Unknown frames"
        this.paperGroup.addChild(this.penText)

        this.paperGroup.visible = false

    }

    setFrameId(frameId: number) {
        this.paperGroup.visible = true
        let penBounds = this.paperGroup.bounds

        this.paperGroup.position = new paper.Point(penOffset + (frameId - this.minFrameId) * this.cellWidth, penBounds.height / 2.0)
        this.penText.content = frameId + 1 // The frameId starts from 0 internall, but during display, it starts from 1.

        this.frameId = frameId
    }

    get paperHeight() {
        return this.paperGroup.bounds.height
    }

    remove() {
        this.paperGroup.remove()
    }
}

class TimelineSpan {
    rectangleShape: paper.Path
    cellWidth: number = -1
    yOffset: number = -1

    startFrameId: number = -1
    endFrameId: number = -1

    constructor(yOffset) {
        this.rectangleShape = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Size(10, 10))
        this.rectangleShape.fillColor = new paper.Color("lightblue")
        this.rectangleShape.strokeColor = new paper.Color("black")
        this.rectangleShape.strokeWidth = 2
        this.yOffset = yOffset

        this.rectangleShape.visible = false
    }

    pointOverRectangle(point: paper.Point) {
        if (this.rectangleShape.contains(point)) {
            this.rectangleShape.fillColor = new paper.Color("darkblue")
            this.rectangleShape.strokeColor = new paper.Color("gray")
            return true
        } else {
            this.rectangleShape.fillColor = new paper.Color("lightblue")
            this.rectangleShape.strokeColor = new paper.Color("black")

            return false
        }
    }

    setFrameSpan(startFrameId, endFrameId) {
        this.startFrameId = startFrameId
        this.endFrameId = endFrameId

        let left = penOffset + startFrameId * this.cellWidth
        let right = penOffset + endFrameId * this.cellWidth

        // LeftUp
        this.rectangleShape.segments[0].point = new paper.Point(left, this.yOffset)
        // RightUp
        this.rectangleShape.segments[1].point = new paper.Point(right, this.yOffset)
        // RightDown
        this.rectangleShape.segments[2].point = new paper.Point(right, this.yOffset + spanRectHeight)
        // LeftDown
        this.rectangleShape.segments[3].point = new paper.Point(left, this.yOffset + spanRectHeight)

        this.rectangleShape.visible = true
    }

    remove() {
        this.rectangleShape.remove()
    }
}

type IntArrayProps = {
    width: number,
    height: number
    property: object
}

class IntArrayX extends React.Component<IntArrayProps, any> {
    projectId: number = -1
    bgRectangle
    canvasRef

    curHighlighedSpan: TimelineSpan = null
    timelineSpans: TimelineSpan[] = new Array()
    contextMenu: ContextMenu = new ContextMenu()
    timelinePointers: TimelinePointer[] = new Array()
    selectedPointer = null

    constructor(props) {
        super(props);
        this.canvasRef = React.createRef()
    }

    getTimelinePointer(idx: number): TimelinePointer {
        while (this.timelinePointers.length <= idx) {
            let newTimelinePointer = new TimelinePointer()
            this.timelinePointers.push(newTimelinePointer)
            newTimelinePointer.paperGroup.onClick = this.onPenClicked.bind(this)
        }

        return this.timelinePointers[idx]
    }

    getTimelineSpan(idx: number, yOffset: number): TimelineSpan {
        while (this.timelineSpans.length <= idx) {
            this.timelineSpans.push(new TimelineSpan(yOffset))
        }

        return this.timelineSpans[idx]
    }

    onPenClicked(evt: MouseEvent) {
        console.log(evt)
        if (!evt.target["data"] || !evt.target["data"]["meta"]) {
            return
        }

        let timelinePointer = evt.target["data"]["meta"] as TimelinePointer
        huahuoEngine.getActivePlayer().setFrameId(timelinePointer.frameId)

        if (this.selectedPointer != null)
            this.selectedPointer.selected = false

        this.selectedPointer = timelinePointer
        this.selectedPointer.selected = true

        triggerFocus(this.canvasRef.current)
    }

    refresh() {
        let property = this.props.property
        let oldProjectId = paper.project.index

        try {
            paper.projects[this.projectId].activate()

            // @ts-ignore
            let intArrayValues = property.getter()

            let minValue = Number.MAX_VALUE
            let maxValue = -1

            for (let frameId of intArrayValues) {
                if (minValue > frameId)
                    minValue = frameId
                if (maxValue < frameId)
                    maxValue = frameId
            }

            let index = 0
            let cellWidth = (this.props.width - 2 * penOffset) / (maxValue - minValue + 1)

            let firstSpanFrameId = -1
            let lastSpanFrameId = -1
            for (let frameId of intArrayValues) {

                if (firstSpanFrameId < 0) {
                    firstSpanFrameId = frameId
                }

                // Draw timeline pointer
                let timelinePointer = this.getTimelinePointer(index)
                timelinePointer.cellWidth = cellWidth
                timelinePointer.minFrameId = minValue
                timelinePointer.setFrameId(frameId)

                // Draw timeline span
                if (lastSpanFrameId >= 0) {
                    let span = this.getTimelineSpan(index, timelinePointer.paperHeight)
                    span.cellWidth = cellWidth
                    span.setFrameSpan(lastSpanFrameId - firstSpanFrameId, frameId - firstSpanFrameId)
                }

                if (huahuoEngine.getActivePlayer().currentlyPlayingFrameId == frameId) {
                    // timelinePointer.selected = true
                    // this.selectedPointer = timelinePointer
                } else {
                    timelinePointer.selected = false
                }

                lastSpanFrameId = frameId
                index++
            }

            for (let unusedSpanIndex = index; unusedSpanIndex < this.timelineSpans.length; unusedSpanIndex++) {
                let span = this.timelineSpans.pop()
                span.remove()
            }

            for (let unusedTimelineIndex = index; unusedTimelineIndex < this.timelinePointers.length; unusedTimelineIndex++) {
                let pointer = this.timelinePointers.pop()
                pointer.remove()
            }
        } finally {
            paper.projects[oldProjectId].activate()
        }
    }

    componentDidMount() {
        if (this.projectId == -1) {
            let previousProject = paper.project
            paper.setup(this.canvasRef.current)
            this.projectId = paper.project.index

            this.bgRectangle = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(this.props.width, this.props.height))
            this.bgRectangle.fillColor = new paper.Color("lightgray")
            this.refresh()
            previousProject.activate();
        }
    }

    kbEventAttached = false

    onKeyUp(evt: KeyboardEvent) {
        if (evt.code == "Delete") {
            if (this.timelinePointers.length <= 1) {
                HHToast.warn(i18n.t("toast.insufficientKeyFrames"))
            } else {
                let property = this.props.property
                let tobeDeletedPointer = this.selectedPointer

                this.selectedPointer = null

                this.timelinePointers = this.timelinePointers.filter((penInArray) => {
                    return penInArray.frameId != tobeDeletedPointer.frameId
                })

                // @ts-ignore
                property.deleter(tobeDeletedPointer.frameId)

                tobeDeletedPointer.remove()

                this.refresh()

                evt.stopPropagation()
            }
        }
    }

    offsetX(mouseEvent) {
        // @ts-ignore
        const rect = mouseEvent.target.getBoundingClientRect()
        return mouseEvent.clientX - rect.left
    }

    offsetY(mouseEvent) {
        // @ts-ignore
        const rect = mouseEvent.target.getBoundingClientRect()
        return mouseEvent.clientY - rect.top
    }

    onKeyUpEvent = this.onKeyUp.bind(this)

    render() {
        let property = this.props.property

        return (
            <canvas ref={this.canvasRef} width={this.props.width} height={this.props.height} style={{
                width: this.props.width,
                height: this.props.height
            }} tabIndex={0} onFocus={() => {
                if (this.kbEventAttached == false) {
                    document.addEventListener("keyup", this.onKeyUpEvent)
                    this.kbEventAttached = true
                }
            }} onBlur={() => {
                document.removeEventListener("keyup", this.onKeyUpEvent)
            }} onMouseMove={(evt) => {
                this.curHighlighedSpan = null
                let oldProjectId = paper.project.index
                try {
                    paper.projects[this.projectId].activate()
                    for (let spanIdx = 0; spanIdx < this.timelineSpans.length; spanIdx++) {
                        let worldPos = paper.view.viewToProject(new paper.Point(this.offsetX(evt), this.offsetY(evt)))
                        let targetSpan = this.timelineSpans[spanIdx]
                        if (targetSpan.pointOverRectangle(worldPos)) {
                            this.curHighlighedSpan = targetSpan
                        }
                    }
                } finally {
                    paper.projects[oldProjectId].activate()
                }
            }} onClick={(evt) => {
                if (this.curHighlighedSpan) {
                    this.contextMenu.setItems([
                        {
                            itemName: "Reverse Span",
                            onclick: () => {
                                let startFrameId = this.curHighlighedSpan.startFrameId
                                let endFrameId = this.curHighlighedSpan.endFrameId

                                // @ts-ignore
                                property.updater("ReverseKeyFrames", {startFrameId: startFrameId, endFrameId: endFrameId})
                            }
                        }
                    ])

                    this.contextMenu.onContextMenu(evt)
                }
            }}>

            </canvas>
        );
    }
}

export {IntArrayX}