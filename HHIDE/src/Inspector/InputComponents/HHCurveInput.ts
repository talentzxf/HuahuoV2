import {CustomElement} from "hhcommoncomponents";
import {ContextMenu} from "hhcommoncomponents";
import {Vector2} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {TransformHandlerMap} from "../../TransformHandlers/TransformHandlerMap";
import {BaseShapeDrawer} from "../../ShapeDrawers/BaseShapeDrawer";
import {ShapeTranslateMorphBase} from "../../TransformHandlers/ShapeTranslateMorphBase";

declare class KeyFrameCurvePoint {
    GetValue(): number

    GetFrameId(): number

    GetHandleIn(): Vector2

    GetHandleOut(): Vector2
}

class ViewPort {
    canvasWidth
    canvasHeight

    viewWidth
    viewHeight

    viewXMin
    viewYMin
    viewXMax
    viewYMax
    leftDown

    get viewXSpan() {
        return this.viewXMax - this.viewXMin
    }

    get viewYSpan() {
        return this.viewYMax - this.viewYMin
    }

    viewPointToCanvasPoint(p) {
        let [x, y] = this.viewToCanvas(p.x, p.y)
        return new paper.Point(x, y)
    }

    viewToCanvas(x, y) {
        let xScale = this.viewWidth / this.viewXSpan
        let yScale = this.viewHeight / this.viewYSpan

        let canvasX = this.leftDown[0] + (x - this.viewXMin) * xScale
        let canvasY = this.leftDown[1] - (y - this.viewYMin) * yScale

        return [canvasX, canvasY]
    }
}

class AxisSystem {
    originPoint: paper.Point = new paper.Point(0, 0)
    originCircle
    xAxisLine
    yAxisLine

    viewPort: ViewPort

    constructor(viewPort: ViewPort) {
        this.viewPort = viewPort
        // this.originCircle = new paper.Path.Circle(this.originPoint, 10)
        // this.originCircle.applyMatrix = false
        // this.originCircle.fillColor = new paper.Color("yellow")

        let xAxis = new paper.Point(0, 0)
        let yAxis = new paper.Point(0, 0)
        this.xAxisLine = new paper.Path.Line(this.originPoint, xAxis)
        this.yAxisLine = new paper.Path.Line(this.originPoint, yAxis)

        this.xAxisLine.strokeColor = new paper.Color("black")
        this.yAxisLine.strokeColor = new paper.Color("black")
    }

    setOriginPosition(x, y) {
        this.originPoint.x = x
        this.originPoint.y = y

        // this.originCircle.position = this.viewPort.viewPointToCanvasPoint(this.originPoint)
    }

    setXLength(xLength) {
        this.xAxisLine.segments[0].setPoint(this.viewPort.viewPointToCanvasPoint(this.originPoint))
        this.xAxisLine.segments[1].setPoint(this.viewPort.viewPointToCanvasPoint(new paper.Point(this.originPoint.x + xLength, this.originPoint.y)))
    }

    setYLength(yLength) {
        this.yAxisLine.segments[0].setPoint(this.viewPort.viewPointToCanvasPoint(this.originPoint))
        this.yAxisLine.segments[1].setPoint(this.viewPort.viewPointToCanvasPoint(new paper.Point(this.originPoint.x, this.originPoint.y + yLength)))
    }
}

@CustomElement({
    selector: "hh-curve-input"
})
class HHCurveInput extends HTMLElement {
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D

    infoPrompt: HTMLDivElement

    projectId: number = -1

    infoPromptContextMenu: ContextMenu = new ContextMenu

    bgRectangle

    viewPort: ViewPort = new ViewPort()

    keyFrameCurveGetter: Function

    axisSystem: AxisSystem
    hitOptions = {}
    transformHandlerMap: TransformHandlerMap
    transformHandler: ShapeTranslateMorphBase = null

    constructor(keyFrameCurveGetter) {
        super();

        this.keyFrameCurveGetter = keyFrameCurveGetter

        this.infoPrompt = document.createElement("div")
        this.infoPrompt.style.position = "absolute"
        this.infoPrompt.className = "tooltip-text"

        this.appendChild(this.infoPrompt)
        this.hideInfoPrompt()

        // Make the position as relative, so infoPrompt can regard HHCurveInput as it's nearest positioned ancestor. https://www.w3.org/TR/css-position-3/
        this.style.position = "relative"

        this.canvas = document.createElement("canvas")
        this.appendChild(this.canvas)

        this.hitOptions = {
            segments: true,
            stroke: true,
            fill: true,
            handles: true,
            tolerance: 5
        }

        this.transformHandlerMap = new TransformHandlerMap()

        this.canvas.onmousedown = this.onMouseDown.bind(this)
    }

    // A simplified version of ShapeSelector logic.
    onMouseDown(evt: MouseEvent) {
        if (evt.buttons != 1)
            return

        let previousProject = paper.project
        try {
            this.activatePaperProject()
            let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
            if (this.transformHandler) {
                this.transformHandler.beginMove(pos)
            } else {
                let hitResultArray = paper.project.hitTestAll(pos, this.hitOptions)
                for (let hitResult of hitResultArray) {
                    if (hitResult && hitResult.item == this.keyFrameCurvePath) {
                        this.keyFrameCurvePath.selected = true
                    } else {
                        this.keyFrameCurvePath.selected = false
                    }
                }
            }
        }
        finally {
            previousProject.activate()
        }
    }

    hideInfoPrompt() {
        this.infoPrompt.style.display = "none"
    }

    showInfoPrompt() {
        this.infoPrompt.style.display = "block"
    }

    connectedCallback() {
        this.refresh()
    }

    activatePaperProject() {
        if (this.projectId < 0) {
            paper.setup(this.canvas)
            this.projectId = paper.project.index

            this.bgRectangle = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(this.canvas.width, this.canvas.height))
            this.bgRectangle.fillColor = new paper.Color("lightgray")

            let contentLayer = new paper.Layer()
            paper.project.addLayer(contentLayer)
            contentLayer.applyMatrix = false
            contentLayer.activate()

            // Draw axis system.
            this.axisSystem = new AxisSystem(this.viewPort)
        }

        paper.projects[this.projectId].activate()
    }

    keyFrameCurvePath: paper.Path

    xAxisTextCache: paper.PointText[] = []
    yAxisTextCache: paper.PointText[] = []
    textSize = 15

    getAxisTextFromCache(textCache: paper.Point[], idx: number) {
        for (let curIdx = textCache.length; curIdx <= idx; curIdx++) {
            let newPointText = new paper.PointText(new paper.Point(0, 0))
            newPointText.justification = "center";
            newPointText.fillColor = new paper.Color("black")
            newPointText.content = "UnSet"
            newPointText.fontSize = this.textSize + "px"
            textCache.push(newPointText)
        }

        return textCache[idx]
    }

    onMouseEnterCircle(evt: paper.MouseEvent) {
        let circle: paper.Item = evt.target
        if (circle.data != null && circle.data.hasOwnProperty("rawObj")) {
            let rawObj = circle.data["rawObj"]

            this.infoPrompt.innerText = i18n.t("inspector.CurveInputPrompt", {
                frameId: rawObj.GetFrameId(),
                value: rawObj.GetValue()
            })
            this.showInfoPrompt()

            // Convert to view coordinate
            let offsetX = evt.point.x
            let offsetY = evt.point.y

            // Check if over the current right border.
            let toolTipBorder = this.infoPrompt.getBoundingClientRect()
            let currentInputBorder = this.getBoundingClientRect()
            if (offsetX + toolTipBorder.width > currentInputBorder.width) {
                offsetX -= toolTipBorder.width;
            }

            this.infoPrompt.style.left = offsetX + "px"
            this.infoPrompt.style.top = offsetY + "px"
        }
    }

    onMouseLeaveCircle(evt: Event) {
        this.hideInfoPrompt()
    }

    circleCache: paper.Path[] = []

    getPaperCircle(idx: number) {
        for (let curIdx = this.circleCache.length; curIdx <= idx; curIdx++) {
            let newCircle = new paper.Path.Circle(new paper.Point(0, 0), 5)
            newCircle.applyMatrix = false
            newCircle.fillColor = new paper.Color("blue")
            newCircle.onMouseEnter = this.onMouseEnterCircle.bind(this)
            newCircle.onMouseLeave = this.onMouseLeaveCircle.bind(this)
            this.circleCache.push(newCircle)
        }

        return this.circleCache[idx]
    }

    refresh() {
        let curve = this.keyFrameCurveGetter()
        if (curve == null)
            return

        let minValue = Number.MAX_VALUE
        let maxValue = -Number.MAX_VALUE

        let minFrameId = Number.MAX_VALUE
        let maxFrameId = -Number.MAX_VALUE

        let points = []
        let totalPoints = curve.GetTotalPoints()
        for (let pointIdx = 0; pointIdx < totalPoints; pointIdx++) {
            let curvePoint = curve.GetKeyFrameCurvePoint(pointIdx)
            let value = curvePoint.GetValue()
            let frameId = curvePoint.GetFrameId() + 1 // In Cpp side, frameId starts from 0. But when shown, frameId starts from 1.

            minValue = Math.min(minValue, value)
            maxValue = Math.max(maxValue, value)
            minFrameId = Math.min(minFrameId, frameId)
            maxFrameId = Math.max(maxFrameId, frameId)

            points.push(curvePoint)
        }

        // Add some offset to avoid 0/0
        if (minFrameId == maxFrameId) {
            maxFrameId = minFrameId + 1
        }

        if (minValue == maxValue) {
            maxValue = minValue + 1
        }

        // Setup port.
        this.viewPort.canvasWidth = this.canvas.width
        this.viewPort.canvasHeight = this.canvas.height
        this.viewPort.viewWidth = 0.8 * this.canvas.width
        this.viewPort.viewHeight = 0.75 * this.canvas.height
        this.viewPort.viewXMin = minFrameId
        this.viewPort.viewXMax = maxFrameId
        this.viewPort.viewYMin = minValue
        this.viewPort.viewYMax = maxValue
        this.viewPort.leftDown = [0.1 * this.canvas.width, 0.8 * this.canvas.height]

        let previousProject = paper.project
        try {
            this.activatePaperProject()

            this.axisSystem.setOriginPosition(minFrameId, minValue)
            this.axisSystem.setXLength(maxFrameId - minFrameId)
            this.axisSystem.setYLength(maxValue - minValue)

            if (this.keyFrameCurvePath) {
                this.keyFrameCurvePath.remove()
            }

            this.keyFrameCurvePath = new paper.Path({
                segments: [],
                strokeColor: 'black',
                fullySelected: true
            })
            this.keyFrameCurvePath.strokeColor = new paper.Color("black")
            this.keyFrameCurvePath.strokeWidth = 3

            let pointIdx = 0
            for (let point of points) {
                let frameId = point.GetFrameId() + 1
                let value = point.GetValue()

                let circle: paper.Path = this.getPaperCircle(pointIdx)

                let keyFramePoint = this.viewPort.viewPointToCanvasPoint(new paper.Point(frameId, value))
                this.keyFrameCurvePath.add(keyFramePoint)

                circle.position = keyFramePoint
                circle.data = {
                    rawObj: point
                }

                // Write x-axis labels.
                let xAxisLabel: paper.PointText = this.getAxisTextFromCache(this.xAxisTextCache, pointIdx)
                xAxisLabel.position = this.viewPort.viewPointToCanvasPoint(new paper.Point(frameId, minValue)).add(new paper.Point(0, this.textSize))
                xAxisLabel.content = String(frameId)

                // Write y-axis labels.
                let yAxisLabel: paper.PointText = this.getAxisTextFromCache(this.yAxisTextCache, pointIdx)
                yAxisLabel.position = this.viewPort.viewPointToCanvasPoint(new paper.Point(minFrameId, value)).subtract(new paper.Point(this.textSize, 0))
                yAxisLabel.content = parseFloat(value.toFixed(2)).toString()

                pointIdx++
            }

        } finally {
            previousProject.activate()
        }
    }

}

export {HHCurveInput}