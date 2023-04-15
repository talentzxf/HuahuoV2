import {CustomElement} from "hhcommoncomponents";
import {ContextMenu} from "hhcommoncomponents";
import {Vector2} from "hhcommoncomponents";
import {paper} from "hhenginejs"
import {BaseShapeDrawer} from "../../ShapeDrawers/BaseShapeDrawer";
import {
    shapeHandlerMoveHandler,
    shapeInsertSegmentHandler,
    shapeMorphHandler,
    ShapeMorphHandler
} from "../../TransformHandlers/ShapeMorphHandler";
import {switchPaperProject} from "./Utils";
import {ViewPort} from "./ViewPort";
import {MovableCurve} from "./MovableCurve";
import {AxisSystem} from "./AxisSystem";
declare class KeyFrameCurvePoint {
    GetValue(): number

    GetFrameId(): number

    GetHandleIn(): Vector2

    GetHandleOut(): Vector2
}

const defaultCanvasWidth = 300
const defaultCanvasHeight = 150

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
    transformHandlerMap: {}
    transformHandler: ShapeMorphHandler = null

    minValue = Number.MAX_VALUE
    maxValue = -Number.MAX_VALUE

    minFrameId = Number.MAX_VALUE
    maxFrameId = -Number.MAX_VALUE

    constructor(keyFrameCurveGetter) {
        super();

        this.transformHandlerMap = {
            "segment": shapeMorphHandler,
            "handle-in": shapeHandlerMoveHandler,
            "handle-out": shapeHandlerMoveHandler,
            "stroke": shapeInsertSegmentHandler,
        }

        this.keyFrameCurveGetter = keyFrameCurveGetter

        this.infoPrompt = document.createElement("div")
        this.infoPrompt.style.position = "absolute"
        this.infoPrompt.className = "tooltip-text"
        this.infoPrompt.onmousedown = (evt) => {
            evt.preventDefault()
        }
        this.infoPrompt.oncontextmenu = (evt) => {
            evt.preventDefault()
        }

        this.appendChild(this.infoPrompt)
        this.hideInfoPrompt()

        // Make the position as relative, so infoPrompt can regard HHCurveInput as it's nearest positioned ancestor. https://www.w3.org/TR/css-position-3/
        this.style.position = "relative"

        this.canvas = document.createElement("canvas")
        this.canvas.width = defaultCanvasWidth
        this.canvas.height = defaultCanvasHeight
        this.canvas.style.width = defaultCanvasWidth + "px"
        this.canvas.style.height = defaultCanvasHeight + "px"

        this.appendChild(this.canvas)

        this.hitOptions = {
            segments: true,
            stroke: true,
            fill: true,
            handles: true,
            tolerance: 5
        }

        this.onmousedown = this.onMouseDown.bind(this)
        this.onmousemove = this.onMouseMove.bind(this)
        this.onmouseup = this.onMouseUp.bind(this)
        this.oncontextmenu = this.onContextMenu.bind(this)
    }

    @switchPaperProject
    onContextMenu(evt: PointerEvent) {
        evt.preventDefault()
        let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        let hitResultArray = paper.project.hitTestAll(pos, {
            segments: true,
            stroke: false,
            fill: false,
            handles: false,
            tolerance: 5
        }) // Only check segments

        for (let hitResult of hitResultArray) {
            if (hitResult && hitResult.segment && hitResult.segment.path == this.keyFrameCurvePath) {
                let segment = hitResult.segment
                let index = hitResult.segment.index
                let _this = this
                this.infoPromptContextMenu.setItems([
                    {
                        itemName: i18n.t("inspector.Smooth"),
                        onclick: () => {
                            segment.smooth()
                        }
                    },
                    {
                        itemName: i18n.t("inspector.Sharpen"),
                        onclick: () => {
                            segment.handleIn = 0
                            segment.handleOut = 0
                        }
                    }
                ])

                this.infoPromptContextMenu.onContextMenu(evt)
                break
            }
        }
    }

    @switchPaperProject
    onMouseUp(evt: MouseEvent) {
        if (this.transformHandler) {
            this.transformHandler.endMove()
            this.transformHandler = null
        }
    }

    // This function has side effect, it will change x of pos.
    adjustDraggingPoint(curve, index, pos, mouseOffsetX) {
        let curPoint = curve.GetKeyFrameCurvePoint(index)

        let totalPointCount = curve.GetTotalPoints()

        let isFirstPoint = index == 0
        let isLastPoint = index == totalPointCount - 1

        let curFrameId = curPoint.GetFrameId() + 1 // In the view side, always +1

        let leftBoundFrameId = 1 // FrameId can't be smaller than 0
        let rightBoundFrameId = Number.MAX_VALUE

        if(!isFirstPoint){
            let prevPoint = curve.GetKeyFrameCurvePoint(index - 1)
            leftBoundFrameId = prevPoint.GetFrameId() + 1
        }

        if(!isLastPoint){
            let nextPoint = curve.GetKeyFrameCurvePoint(index + 1)
            rightBoundFrameId = nextPoint.GetFrameId() + 1
        }

        pos.x = Math.clamp(mouseOffsetX, this.viewPort.getXOffsetForFrame(leftBoundFrameId), this.viewPort.getXOffsetForFrame(rightBoundFrameId))

    }

    @switchPaperProject
    onMouseMove(evt: MouseEvent) {
        let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        if (this.transformHandler && this.transformHandler.getIsDragging()) {
            let curSegment: paper.Segment = this.transformHandler.getCurSegment()
            let index = curSegment.index

            let curve = this.keyFrameCurveGetter()
            if (curve == null)
                return

            if (this.transformHandler == shapeMorphHandler) // Only shape morph handler need to stick to frame.
            {
                this.adjustDraggingPoint(curve, index, pos, evt.offsetX)
                let [newFrameId, newValue] = this.viewPort.canvasPointToViewPoint(pos.x, pos.y)
                this.showKeyFrameValueIndicator(newFrameId, newValue)
            }

            // Can only be dragged within FrameRange.
            this.transformHandler.dragging(pos)
        }
    }

    getHandler(type) {
        if (this.transformHandlerMap.hasOwnProperty(type)) {
            return this.transformHandlerMap[type]
        }
        return null
    }


    // A simplified version of ShapeSelector logic.
    @switchPaperProject
    onMouseDown(evt: MouseEvent) {
        if (evt.buttons != 1)
            return

        this.hideInfoPrompt()

        let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        if (this.transformHandler) {
            this.transformHandler.beginMove(pos, null, false)
        } else {
            let hitSomething = false
            let hitResultArray = paper.project.hitTestAll(pos, this.hitOptions)
            for (let hitResult of hitResultArray) {
                if (hitResult && hitResult.item == this.keyFrameCurvePath) {
                    hitSomething = true
                    this.keyFrameCurvePath.selected = true
                    this.transformHandler = this.getHandler(hitResult.type)
                    if (this.transformHandler) {
                        let objects = new Set() // Because setTarget need to receive a Set.
                        objects.add(new MovableCurve(this.keyFrameCurvePath))
                        this.transformHandler.setTarget(objects)
                        this.transformHandler.beginMove(pos, hitResult, false)

                        if (hitResult.segment) {
                            let index = hitResult.segment.index
                            let curve = this.keyFrameCurveGetter()
                            let curvePoint = curve.GetKeyFrameCurvePoint(index)
                            let frameIdx = curvePoint.GetFrameId() + 1
                            let value = curvePoint.GetValue()

                            this.showKeyFrameValueIndicator(frameIdx, value)
                        }
                    }
                    break;
                }
            }

            if (!hitSomething) {
                this.keyFrameCurvePath.selected = false
                this.transformHandler = null
            }
        }
    }

    horizontalLine: paper.Path
    verticalLine: paper.Path

    frameValueIndicatorTextX: paper.PointText
    frameValueIndicatorTextY: paper.PointText

    frameValueIndicatorGroup: paper.Group

    showKeyFrameValueIndicator(frameId: number, value: number) {
        if (this.frameValueIndicatorGroup == null) {
            this.frameValueIndicatorGroup = new paper.Group()
        }

        if (this.horizontalLine == null) {
            this.horizontalLine = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(1, 1))
            this.horizontalLine.strokeColor = new paper.Color("red")
            this.horizontalLine.dashArray = [10, 4]

            this.frameValueIndicatorGroup.addChild(this.horizontalLine)
        }

        if (this.verticalLine == null) {
            this.verticalLine = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(1, 1))
            this.verticalLine.strokeColor = new paper.Color("red")
            this.verticalLine.dashArray = [10, 4]
            this.frameValueIndicatorGroup.addChild(this.verticalLine)
        }

        this.horizontalLine.segments[0].point = this.viewPort.viewPointToCanvasPoint(new paper.Point(frameId, value))
        this.horizontalLine.segments[1].point = this.viewPort.viewPointToCanvasPoint(new paper.Point(this.minFrameId, value))

        this.verticalLine.segments[0].point = this.viewPort.viewPointToCanvasPoint(new paper.Point(frameId, value))
        this.verticalLine.segments[1].point = this.viewPort.viewPointToCanvasPoint(new paper.Point(frameId, this.minValue))

        if (this.frameValueIndicatorTextX == null) {
            this.frameValueIndicatorTextX = this.createPointText()
            this.frameValueIndicatorGroup.addChild(this.frameValueIndicatorTextX)

            this.frameValueIndicatorTextX.fontSize = this.smallTextSize
        }
        this.frameValueIndicatorTextX.content = parseFloat(frameId.toFixed(0)).toString()
        this.frameValueIndicatorTextX.position = this.viewPort.viewPointToCanvasPoint(new paper.Point(frameId, this.minValue)).add(new paper.Point(0, this.textSize))

        if (this.frameValueIndicatorTextY == null) {
            this.frameValueIndicatorTextY = this.createPointText()
            this.frameValueIndicatorGroup.addChild(this.frameValueIndicatorTextY)
        }
        this.frameValueIndicatorTextY.content = parseFloat(value.toFixed(2)).toString()
        this.frameValueIndicatorTextY.position = this.viewPort.viewPointToCanvasPoint(new paper.Point(this.minFrameId, value)).subtract(new paper.Point(this.textSize, 0))

        this.frameValueIndicatorGroup.visible = true
    }

    hideKeyFrameValueIndicator() {
        if(this.frameValueIndicatorGroup)
            this.frameValueIndicatorGroup.visible = false
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
    smallTextSize = 10

    createPointText() {
        let pointText = new paper.PointText(new paper.Point(0, 0))
        pointText.justification = "center";
        pointText.fillColor = new paper.Color("black")
        pointText.content = "UnSet"
        pointText.fontSize = this.textSize + "px"
        return pointText
    }

    getAxisTextFromCache(textCache: paper.Point[], idx: number) {
        for (let curIdx = textCache.length; curIdx <= idx; curIdx++) {
            textCache.push(this.createPointText())
        }

        return textCache[idx]
    }

    onMouseEnterCircle(evt: paper.MouseEvent) {
        let circle: paper.Item = evt.target
        if (circle.data != null && circle.data.hasOwnProperty("rawObj")) {
            let rawObj = circle.data["rawObj"]

            this.infoPrompt.innerHTML = i18n.t("inspector.CurveInputPrompt", {
                frameId: rawObj.GetFrameId() + 1,
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

    @switchPaperProject
    refresh() {
        let curve = this.keyFrameCurveGetter()
        if (curve == null)
            return

        this.hideKeyFrameValueIndicator()

        let points = []
        let totalPoints = curve.GetTotalPoints()
        for (let pointIdx = 0; pointIdx < totalPoints; pointIdx++) {
            let curvePoint = curve.GetKeyFrameCurvePoint(pointIdx)
            let value = curvePoint.GetValue()
            let frameId = curvePoint.GetFrameId() + 1 // In Cpp side, frameId starts from 0. But when shown, frameId starts from 1.

            this.minValue = Math.min(this.minValue, value)
            this.maxValue = Math.max(this.maxValue, value)
            this.minFrameId = Math.min(this.minFrameId, frameId)
            this.maxFrameId = Math.max(this.maxFrameId, frameId)

            points.push(curvePoint)
        }

        // Add some offset to avoid 0/0
        if (this.minFrameId == this.maxFrameId) {
            this.maxFrameId = this.minFrameId + 1
        }

        if (this.minValue == this.maxValue) {
            this.maxValue = this.minValue + 1
        }

        // Setup port.
        this.viewPort.canvasWidth = defaultCanvasWidth
        this.viewPort.canvasHeight = defaultCanvasHeight
        this.viewPort.viewWidth = 0.8 * defaultCanvasWidth
        this.viewPort.viewHeight = 0.75 * defaultCanvasHeight
        this.viewPort.viewXMin = this.minFrameId
        this.viewPort.viewXMax = this.maxFrameId
        this.viewPort.viewYMin = this.minValue
        this.viewPort.viewYMax = this.maxValue
        this.viewPort.leftDown = [0.1 * defaultCanvasWidth, 0.8 * defaultCanvasHeight]


        this.axisSystem.setOriginPosition(this.minFrameId, this.minValue)
        this.axisSystem.setXLength(this.maxFrameId - this.minFrameId)
        this.axisSystem.setYLength(this.maxValue - this.minValue)

        if (this.keyFrameCurvePath) {
            this.keyFrameCurvePath.remove()
        }

        this.keyFrameCurvePath = new paper.Path({
            segments: [],
            strokeColor: 'black'
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
            xAxisLabel.position = this.viewPort.viewPointToCanvasPoint(new paper.Point(frameId, this.minValue)).add(new paper.Point(0, this.textSize))
            xAxisLabel.content = String(frameId)

            // Write y-axis labels.
            let yAxisLabel: paper.PointText = this.getAxisTextFromCache(this.yAxisTextCache, pointIdx)
            yAxisLabel.position = this.viewPort.viewPointToCanvasPoint(new paper.Point(this.minFrameId, value)).subtract(new paper.Point(this.textSize, 0))
            yAxisLabel.content = parseFloat(value.toFixed(2)).toString()

            pointIdx++
        }

        this.keyFrameCurvePath.bringToFront()
    }

}

export {HHCurveInput}