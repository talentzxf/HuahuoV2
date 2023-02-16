import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {createPenShape, selectedPenCapColor, unselectedPenCapColor} from "./Utils";
import {huahuoEngine} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {ContextMenu} from "hhcommoncomponents";

const canvasWidth = 200
const canvasHeight = 50
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

@CustomElement({
    selector: "hh-int-array-input"
})
class HHIntArray extends HTMLElement implements RefreshableComponent {
    getter: Function
    setter: Function
    updater: Function
    deleter: Function
    titleDiv: HTMLDivElement

    isCollapsed: boolean = true
    collapseButton: HTMLInputElement = null

    canvas: HTMLCanvasElement = null
    projectId: number = -1

    bgRectangle: paper.Path = null

    timelinePointers: TimelinePointer[] = new Array()
    timelineSpans: TimelineSpan[] = new Array()

    selectedPointer = null

    kbEventAttached = false

    curHighlighedSpan: TimelineSpan = null

    contextMenu: ContextMenu = new ContextMenu()

    constructor(getter, setter, updater, deleter, titleDiv) {
        super();

        this.titleDiv = titleDiv
        this.getter = getter
        this.updater = updater
        this.deleter = deleter
        this.setter = setter

        this.collapseButton = document.createElement("input")
        this.collapseButton.type = "button"

        this.collapseTimeline()

        titleDiv.appendChild(this.collapseButton)

        this.canvas = document.createElement("canvas")
        this.canvas.width = canvasWidth
        this.canvas.height = canvasHeight
        this.canvas.style.width = canvasWidth + "px"
        this.canvas.style.height = canvasHeight + "px"
        this.appendChild(this.canvas)

        let previousProject = paper.project
        paper.setup(this.canvas)
        this.projectId = paper.project.index

        this.bgRectangle = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(canvasWidth, canvasHeight))
        this.bgRectangle.fillColor = new paper.Color("lightgray")

        this.refresh()

        previousProject.activate()

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

        this.addEventListener("mousemove", (evt: MouseEvent) => {
            this.curHighlighedSpan = null
            let oldProjectId = paper.project.index

            try {
                paper.projects[this.projectId].activate()
                for (let spanIdx = 0; spanIdx < this.timelineSpans.length; spanIdx++) {
                    let worldPos = paper.view.viewToProject(new paper.Point(evt.offsetX, evt.offsetY))
                    let targetSpan = this.timelineSpans[spanIdx]
                    if (targetSpan.pointOverRectangle(worldPos)) {
                        this.curHighlighedSpan = targetSpan
                    }
                }
            } finally {
                paper.projects[oldProjectId].activate()
            }
        })

        this.addEventListener("click", (evt: MouseEvent) => {
            if (this.curHighlighedSpan) {
                this.contextMenu.setItems([
                    {
                        itemName: "Reverse Span",
                        onclick: () => {
                            let startFrameId = this.curHighlighedSpan.startFrameId
                            let endFrameId = this.curHighlighedSpan.endFrameId

                            this.updater("ReverseKeyFrames", {startFrameId: startFrameId, endFrameId: endFrameId})
                        }
                    }
//                 {
//                     itemName: "Create Cube",
//                     onclick: _this.treeEventHandler.createCube.bind(_this.treeEventHandler)
//                 },
//                 {
//                     itemName: "Delete Object",
//                     onclick: () => {
//                         alert("Delete object")
//                     }
                ])

                this.contextMenu.onContextMenu(evt)
            }
        })
    }

    onKeyUp(evt: KeyboardEvent) {
        if (evt.code == "Delete") {
            if (this.timelinePointers.length <= 2) {
                HHToast.warn(i18n.t("toast.insufficientKeyFrames"))
            } else {
                let tobeDeletedPointer = this.selectedPointer

                this.selectedPointer = null

                this.timelinePointers = this.timelinePointers.filter((penInArray) => {
                    return penInArray.frameId != tobeDeletedPointer.frameId
                })

                this.deleter(tobeDeletedPointer.frameId)

                tobeDeletedPointer.remove()

                this.refresh()

                evt.stopPropagation()
            }
        }
    }

    openTimeline() {
        this.collapseButton.value = "-"
        this.collapseButton.onclick = this.collapseTimeline.bind(this)

        this.style.display = "block"

        this.refresh()

        triggerFocus(this)
    }

    collapseTimeline() {
        this.collapseButton.value = "+"
        this.collapseButton.onclick = this.openTimeline.bind(this)

        this.style.display = "none"
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
        timelinePointer.selected = true
        this.selectedPointer = timelinePointer

        triggerFocus(this)
    }

    refresh() {
        let oldProjectId = paper.project.index

        try {
            paper.projects[this.projectId].activate()

            let intArrayValues = this.getter()

            let minValue = Number.MAX_VALUE
            let maxValue = -1

            for (let frameId of intArrayValues) {
                if (minValue > frameId)
                    minValue = frameId
                if (maxValue < frameId)
                    maxValue = frameId
            }

            let index = 0
            let cellWidth = (canvasWidth - 2 * penOffset) / (maxValue - minValue + 1)

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
}

export {HHIntArray}