import * as paper from "paper"
import {view} from "paper";
import {Logger, Vector2} from "hhcommoncomponents"
import {RenderEngine2D} from "./RenderEngine2D";
import {huahuoEngine} from "../EngineAPI";
import {GraphEvent, EventParam, PropertyType} from "hhcommoncomponents";
import {EventEmitter} from "hhcommoncomponents";

let bgLayerName = "background"
let contentLayerName = "content"

class RenderEnginePaperJs extends EventEmitter implements RenderEngine2D {

    // From canvas to project index
    private canvasPaperMap: Map<HTMLCanvasElement, number> = new Map()
    private canvasOriginalSize: Map<paper.View, [number, number]> = new Map()

    private originalZoomMap: Map<paper.View, number> = new Map()

    private originalViewPosition: Map<paper.View, paper.Point> = new Map()

    private isPlayer = false
    private aspectRatio: number = 4 / 3  //  W:H = 4:3

    getAspectRatio(){
        return this.aspectRatio
    }

    getEventEmitterName() {
        return "Canvas"
    }

    getInitCanvasWH(): [number, number] {
        return [huahuoEngine.getProjectWidth(), huahuoEngine.getProjectHeight()]
    }

    getLayerByName(layerName) {
        let project = this.getProject()
        let targetLayers = project.layers.filter((layer) => {
            return layer.name == layerName
        })
        if (targetLayers.length != 1)
            return null
        return targetLayers[0]
    }

    get bgLayer() {
        return this.getLayerByName(bgLayerName)
    }

    get contentLayer() {
        return this.getLayerByName(contentLayerName)
    }

    private getProject() {
        return paper.project
    }

    private activateBgLayer() {
        if (!this.bgLayer) {
            let contentLayer = this.getProject().activeLayer
            contentLayer.name = contentLayerName
            let bgLayer = new paper.Layer();
            bgLayer.name = "background"
            this.getProject().insertLayer(0, bgLayer)
        }
        this.bgLayer.activate()
    }

    private restoreContentLayer() {
        this.contentLayer.activate()
    }

    private createViewRectangle(x, y, w, h, fillColor) {
        this.activateBgLayer()
        // Convert all points from view->project coordinate
        let leftUp = view.viewToProject(new paper.Point(x, y))
        let rightDown = view.viewToProject(new paper.Point(x + w, y + h))

        return new paper.Path.Rectangle({
            point: [leftUp.x, leftUp.y],
            size: [rightDown.x - leftUp.x, rightDown.y - leftUp.y],
            strokeColor: 'white',
            fillColor: fillColor,
            selected: false
        })
    }

    setBgColor(bgColor) {
        this.bgFillColor = new paper.Color(bgColor.red, bgColor.green, bgColor.blue, bgColor.alpha)
        this.bgRectangle.fillColor = this.bgFillColor
    }

    getBgColor() {
        return this.bgFillColor
    }

    public getContentWH(canvasWidth, canvasHeight): [number, number] {
        let returnWidth = canvasWidth
        let returnHeight = canvasWidth / this.aspectRatio
        if (returnHeight > canvasHeight) {
            returnHeight = canvasHeight
            returnWidth = canvasHeight * this.aspectRatio
        }

        return [returnWidth, returnHeight]
    }

    public zoomIn(step = 1.05) {
        view.zoom *= step;
        Logger.debug("Current view zoom:" + view.zoom)
    }

    public zoomOut(step = 1.05) {
        view.zoom /= step;
        Logger.debug("Current view zoom:" + view.zoom)
    }

    public zoomReset() {
        if (this.originalZoomMap.has(view)) {
            view.zoom = this.originalZoomMap.get(view)
        } else {
            view.zoom = 1.0
        }

        if (this.originalViewPosition.has(view)) {
            view.center = this.originalViewPosition.get(view)
        } else {
            view.center = new paper.Point(0, 0)
        }
    }

    bgFillColor = new paper.Color("white")
    bgRectangle

    public clearBackground() {

        let canvasWidth = view.element.width
        let canvasHeight = view.element.height

        // Logger.debug("Canvas width:" + canvasWidth + ",height:" + canvasHeight)

        if (this.bgLayer) {
            this.bgLayer.removeChildren()
        }

        let [contentWidth, contentHeight] = this.getContentWH(canvasWidth, canvasHeight)
        let xOffset = (canvasWidth - contentWidth) / 2
        let yOffset = (canvasHeight - contentHeight) / 2
        this.bgRectangle = this.createViewRectangle(xOffset, yOffset, contentWidth, contentHeight, this.bgFillColor)
        this.bgRectangle.sendToBack()
        this.restoreContentLayer()
    }

    public getWorldPosFromView(x: number, y: number): Vector2 {
        let worldPos = paper.view.viewToProject(new paper.Point(x, y))
        return new Vector2(worldPos.x, worldPos.y)
    }

    public init(canvas: HTMLCanvasElement, isPlayer: boolean = false) {
        console.log("Initing paper!!!!")
        if (this.canvasPaperMap.get(canvas)) {
            console.log("Already inited, won't init twice!")
            return
        }

        this.isPlayer = isPlayer
        paper.setup(canvas)

        window.paper = paper

        this.clearBackground()

        this.canvasPaperMap.set(canvas, paper.project.index)

        view.onMouseMove = (evt: paper.MouseEvent) => {
            this.onMouseMove(evt.point)
        }

        view.onMouseDown = (evt: paper.MouseEvent) => {
            this.onMouseDown(evt.point)
        }

        view.onMouseUp = (evt: paper.MouseEvent) => {
            this.onMouseUp(evt.point)
        }
    }

    @GraphEvent(true)
    onMouseMove(@EventParam(PropertyType.VECTOR2) point: paper.Point) {

    }

    @GraphEvent(true)
    onMouseDown(@EventParam(PropertyType.VECTOR2) point: paper.Point) {

    }

    @GraphEvent(true)
    onMouseUp(@EventParam(PropertyType.VECTOR2) point: paper.Point) {

    }

    saveProjectCanvasWH(width, height) {
        huahuoEngine.setProjectWidthHeight(width, height)
    }

    resize(canvas: HTMLCanvasElement, width: number, height: number) {

        if (!this.canvasPaperMap.has(canvas)) {
            Logger.error("canvas not registered!!!")
            return
        }

        let canvasPaperIdx = this.canvasPaperMap.get(canvas)
        let canvasPaper = paper.projects[canvasPaperIdx]
        let canvasView = canvasPaper.view

        if (!this.canvasOriginalSize.has(canvasView)) {
            if (!this.isPlayer) {
                if (width > 0 && height > 0) {
                    console.log("Added canvas original size here:" + width)
                    this.canvasOriginalSize.set(canvasView, [width, height])

                    if (this.getInitCanvasWH()[0] < 0) {
                        huahuoEngine.setProjectWidthHeight(width, height)
                    }
                }
            } else {
                let originalSize = this.getInitCanvasWH()
                if (originalSize[0] > 0) {
                    this.canvasOriginalSize.set(canvasView, originalSize)

                    this._resize(width, height, originalSize, canvasView)
                }
            }
        } else {
            let originalSize = this.canvasOriginalSize.get(canvasView)

            this._resize(width, height, originalSize, canvasView)
        }

        this.clearBackground()

        this.originalZoomMap.set(canvasView, canvasView.zoom)
        this.originalViewPosition.set(canvasView, canvasView.center)
    }

    private _resize(width, height, originalSize, canvasView: paper.View) {
        let currentContentDim = this.getContentWH(width, height)
        let currentX = (width - currentContentDim[0]) / 2
        let currentY = (height - currentContentDim[1]) / 2

        let ratio = currentContentDim[0] / originalSize[0]

        canvasView.scaling.x = ratio
        canvasView.scaling.y = ratio

        console.log("originalWidth:" + originalSize[0] + ", current width:" + currentContentDim[0] + " Ratio:" + ratio)

        let projectOriginPos = canvasView.projectToView(new paper.Point(0, 0)) // Current origin in view coordinate

        let offset = new paper.Point(-projectOriginPos.x / ratio, -projectOriginPos.y / ratio)
        canvasView.translate(offset)
        canvasView.translate(new paper.Point(currentX / ratio, currentY / ratio))

        // @ts-ignore
        canvasView.setViewSize(width, height)
    }

    public getDefaultCanvas() {
        let view: any = paper.project.view
        let originalActiveCanvas = view.getElement()
        return originalActiveCanvas
    }

    public setDefaultCanvas(canvas: HTMLCanvasElement) {
        if (this.canvasPaperMap.has(canvas)) {
            let originalActiveCanvas = this.getDefaultCanvas()

            if (originalActiveCanvas == canvas)
                return;

            let projectIndex = this.canvasPaperMap.get(canvas)
            paper.projects[projectIndex].activate()

            window.paper = paper

            this.clearBackground()

            if (this.isPlayer) {
                let _this = this
                huahuoEngine.ExecuteAfterInited(() => {
                    _this.resize(canvas, canvas.width, canvas.height)
                })
            }

            return originalActiveCanvas
        }
        return null
    }

    public getGlobalPosition(viewX: number, viewY: number): paper.Point {
        return this.getProject().view.viewToProject(new paper.Point(viewX, viewY))
    }
}

export {RenderEnginePaperJs}