import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {createPenShape, selectedPenCapColor, unselectedPenCapColor} from "./Utils";
import {huahuoEngine} from "hhenginejs";

const canvasWidth = 200
const canvasHeight = 50
const penOffset = 10
const textMargin = 5
const spanRectHeight = 5

class TimelinePointer {
    paperGroup: paper.Group
    penCap: paper.Path
    penBody: paper.Path
    penText: paper.PointText
    _selected: boolean = false

    frameId: number = -1
    cellWidth: number = -1
    minFrameId: number = -1

    set selected(val: boolean){
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

    get paperHeight(){
        return this.paperGroup.bounds.height
    }
}

class TimelineSpan{
    rectangleShape: paper.Path
    cellWidth: number = -1
    yOffset: number = -1
    constructor(yOffset) {
        this.rectangleShape = new paper.Path.Rectangle(new paper.Point(0,0), new paper.Size(10,10))
        this.rectangleShape.fillColor = new paper.Color("lightblue")
        this.rectangleShape.strokeColor = new paper.Color("black")
        this.rectangleShape.strokeWidth = 2
        this.yOffset = yOffset

        this.rectangleShape.visible = false
    }

    setFrameSpan(startFrameId, endFrameId){
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
    }

    openTimeline() {
        this.collapseButton.value = "-"
        this.collapseButton.onclick = this.collapseTimeline.bind(this)

        this.style.display = "block"

        this.refresh()
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
            newTimelinePointer.paperGroup.onClick = this.onPenClicked
        }

        return this.timelinePointers[idx]
    }

    getTimelineSpan(idx: number, yOffset: number): TimelineSpan{
        while(this.timelineSpans.length <= idx){
            this.timelineSpans.push(new TimelineSpan(yOffset))
        }

        return this.timelineSpans[idx]
    }

    onPenClicked(evt: MouseEvent){
        console.log(evt)
        if(!evt.target["data"] || !evt.target["data"]["meta"]){
            return
        }

        let timelinePointer = evt.target["data"]["meta"] as TimelinePointer
        huahuoEngine.getActivePlayer().setFrameId(timelinePointer.frameId)
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

                if(firstSpanFrameId < 0){
                    firstSpanFrameId = frameId
                }

                // Draw timeline pointer
                let timelinePointer = this.getTimelinePointer(index)
                timelinePointer.cellWidth = cellWidth
                timelinePointer.minFrameId = minValue
                timelinePointer.setFrameId(frameId)

                // Draw timeline span
                if(lastSpanFrameId >= 0){
                    let span = this.getTimelineSpan(index, timelinePointer.paperHeight)
                    span.cellWidth = cellWidth
                    span.setFrameSpan(lastSpanFrameId - firstSpanFrameId, frameId - firstSpanFrameId)
                }

                if(huahuoEngine.getActivePlayer().currentlyPlayingFrameId == frameId){
                    timelinePointer.selected = true
                }else{
                    timelinePointer.selected = false
                }

                lastSpanFrameId = frameId
                index++
            }
        } finally {
            paper.projects[oldProjectId].activate()
        }
    }
}

export {HHIntArray}