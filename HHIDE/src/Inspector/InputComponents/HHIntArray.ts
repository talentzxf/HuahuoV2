import {CustomElement} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {createPenShape, penHeight} from "./Utils";

const canvasWidth = 200
const canvasHeight = 40
const penOffset = 10


class TimelinePointer {
    paperGroup: paper.Group
    penCap: paper.Path
    penBody: paper.Path
    penText: paper.PointText
    selected: boolean = false

    frameId: number = -1
    cellWidth: number = -1

    constructor(cellWidth: number) {
        [this.paperGroup, this.penBody, this.penCap] = createPenShape()

        this.cellWidth = cellWidth

        this.paperGroup.rotation = 180

        this.penText = new paper.PointText(this.penBody.bounds.topLeft)
        this.penText.content = "Unknown frames"
        this.paperGroup.addChild(this.penText)
    }

    setFrameId(frameId: number) {
        this.paperGroup.position = new paper.Point(penOffset + frameId * this.cellWidth, canvasHeight - penHeight)
        this.penText.content = frameId
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

    getTimelinePointer(idx: number, cellWidth: number): TimelinePointer {
        while (this.timelinePointers.length <= idx) {
            this.timelinePointers.push(new TimelinePointer(cellWidth))
        }

        return this.timelinePointers[idx]
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
            for (let frameId of intArrayValues) {
                let timelineIndex = this.getTimelinePointer(index++, cellWidth)
                timelineIndex.setFrameId(frameId)
            }
        } finally {
            paper.projects[oldProjectId].activate()
        }
    }
}

export {HHIntArray}