import {TypedEmitter} from 'tiny-typed-emitter';
import {GlobalConfig} from "hhenginejs";
import {Logger} from "hhcommoncomponents";
import {v4 as uuidv4} from 'uuid';
import {huahuoEngine} from "hhenginejs"

enum TimelineTrackEventNames {
    CELLCLICKED = 'cellClicked'
}

declare class TimeLineTrack {
}

interface TimelineTrackEvent {
    'cellClicked': (track: TimeLineTrack, cellId: number) => void;
}

let ICONWIDTH = 15
let ICONHEIGHT = 15

class TimelineTrack extends TypedEmitter<TimelineTrackEvent> {
    static defaultUnitCellWidth: number = 20;
    static defaultUnitCellHeight: number = 30;

    protected iconWidth: number = ICONWIDTH
    protected iconHeight: number = ICONHEIGHT
    protected unitCellWidth: number = -1;
    protected unitCellHeight: number = -1;
    protected selectable: boolean = true;
    protected cellFontSize: number = 10
    protected cellBgStyle: string = "lightgray"

    private trackNameSize: number = 12
    private ctx: CanvasRenderingContext2D
    private frameCount: number;
    private sequenceId: number

    private layer
    protected cellManager

    private selectedCellStart: number = -1;
    private selectedCellEnd: number = -1;

    private selectedCellBgStyle: string = "cyan"
    private cellStrokeStyle: string = "black"
    private stopFrameStyle: string = "rgb(238, 147, 32, 0.8)"
    private cellFontStyle: string = "black"

    private trackName: string;
    private canvasStartPos: number;
    private canvasEndPos: number;

    private yOffset: number;

    private _elapsedTime: number = 0.0; // In seconds.

    private checkBoxImg = new Image()

    private icons: Array<HTMLImageElement>;
    private onNameClickedCallback: Function = null

    private trackId: uuidv4

    private pendingFuncs: Array<Function> = new Array();

    // Left, Right, onClickFunction
    private iconClickFuncArray: Array<[number, number, Function]> = new Array();
    private iconMouseMoveFuncArray: Array<[number, number, Function]> = new Array();

    public IgnoreDuringLoad() {
        return false
    }

    constructor(sequenceId: number, frameCount: number, ctx: CanvasRenderingContext2D, yOffset = 0, layer = null, trackName: string = "NoNameTrack", frameId: number = -1) {
        super();
        this.sequenceId = sequenceId;
        this.trackName = trackName;
        this.frameCount = frameCount;
        this.ctx = ctx
        this.trackId = uuidv4();

        this.unitCellWidth = TimelineTrack.defaultUnitCellWidth
        this.unitCellHeight = TimelineTrack.defaultUnitCellHeight
        this.yOffset = yOffset

        this.checkBoxImg.src = "data:image/svg+xml;utf8, <svg version=\"1.1\" id=\"Capa_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\t viewBox=\"0 0 512 512\" style=\"enable-background:new 0 0 512 512;\" xml:space=\"preserve\"><g>\t<path d=\"M474.045,173.813c-4.201,1.371-6.494,5.888-5.123,10.088c7.571,23.199,11.411,47.457,11.411,72.1\t\tc0,62.014-24.149,120.315-68,164.166s-102.153,68-164.167,68s-120.316-24.149-164.167-68S16,318.014,16,256\t\tS40.149,135.684,84,91.833s102.153-68,164.167-68c32.889,0,64.668,6.734,94.455,20.017c28.781,12.834,54.287,31.108,75.81,54.315\t\tc3.004,3.239,8.066,3.431,11.306,0.425c3.24-3.004,3.43-8.065,0.426-11.306c-23-24.799-50.26-44.328-81.024-58.047\t\tC317.287,15.035,283.316,7.833,248.167,7.833c-66.288,0-128.608,25.813-175.48,72.687C25.814,127.392,0,189.712,0,256\t\tc0,66.287,25.814,128.607,72.687,175.479c46.872,46.873,109.192,72.687,175.48,72.687s128.608-25.813,175.48-72.687\t\tc46.873-46.872,72.687-109.192,72.687-175.479c0-26.332-4.105-52.26-12.201-77.064\t\tC482.762,174.736,478.245,172.445,474.045,173.813z\"/>\t<path d=\"M504.969,83.262c-4.532-4.538-10.563-7.037-16.98-7.037s-12.448,2.499-16.978,7.034l-7.161,7.161\t\tc-3.124,3.124-3.124,8.189,0,11.313c3.124,3.123,8.19,3.124,11.314-0.001l7.164-7.164c1.51-1.512,3.52-2.344,5.66-2.344\t\ts4.15,0.832,5.664,2.348c1.514,1.514,2.348,3.524,2.348,5.663s-0.834,4.149-2.348,5.663L217.802,381.75\t\tc-1.51,1.512-3.52,2.344-5.66,2.344s-4.15-0.832-5.664-2.348L98.747,274.015c-1.514-1.514-2.348-3.524-2.348-5.663\t\tc0-2.138,0.834-4.149,2.351-5.667c1.51-1.512,3.52-2.344,5.66-2.344s4.15,0.832,5.664,2.348l96.411,96.411\t\tc1.5,1.5,3.535,2.343,5.657,2.343s4.157-0.843,5.657-2.343l234.849-234.849c3.125-3.125,3.125-8.189,0-11.314\t\tc-3.124-3.123-8.189-3.123-11.313,0L212.142,342.129l-90.75-90.751c-4.533-4.538-10.563-7.037-16.98-7.037\t\ts-12.448,2.499-16.978,7.034c-4.536,4.536-7.034,10.565-7.034,16.977c0,6.412,2.498,12.441,7.034,16.978l107.728,107.728\t\tc4.532,4.538,10.563,7.037,16.98,7.037c6.417,0,12.448-2.499,16.977-7.033l275.847-275.848c4.536-4.536,7.034-10.565,7.034-16.978\t\tS509.502,87.794,504.969,83.262z\"/></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g></svg>"

        if (!this.IgnoreDuringLoad()) {
            let _this = this
            huahuoEngine.ExecuteAfterInited(() => {
                if (!layer) { // Create a new layer.
                    layer = huahuoEngine.GetCurrentStore().CreateLayer(_this.getId())
                    layer.SetName(_this.getName())
                    if (frameId >= 0)
                        layer.SetCurrentFrame(frameId)
                } else { // Load from persisted Cpp obj
                    _this.trackName = layer.GetName()
                }
                Logger.debug("New layer created, currently there're:" + huahuoEngine.GetCurrentStore().GetLayerCount() + " layers!")
                _this.setLayer(layer)

                if (this.pendingFuncs.length) {
                    for (let func of this.pendingFuncs) {
                        func()
                    }
                }
            })
        }
    }

    get isSelected() {
        if (this.layer)
            return this.layer.GetIsSelected()
        return false
    }

    set isSelected(val: boolean) {
        if (this.layer)
            this.layer.SetIsSelected(val)
    }

    setLayer(layer) {
        this.layer = layer;
        this.cellManager = layer.GetTimeLineCellManager();
    }

    getLayer() {
        return this.layer
    }

    getName() {
        return this.trackName;
    }

    getId() {
        return this.trackId
    }

    setIcons(icons: Array<HTMLImageElement>) {
        this.icons = icons
    }

    setOnNameClickedCallback(callback: Function) {
        this.onNameClickedCallback = callback
    }

    setElapsedTime(elapsedTime) {
        this._elapsedTime = elapsedTime
    }

    private get elapsedTime() {
        if (this.layer) {
            return (this.layer.GetCurrentFrame() + 0.5) / GlobalConfig.fps
        }

        return this._elapsedTime
    }

    getYOffset(): number {
        return this.yOffset;
    }

    getCellHeight(): number {
        return this.unitCellHeight
    }

    getCellWidth(): number {
        return this.unitCellWidth
    }

    hasYOffset(offsetY: number): boolean {
        if (offsetY >= this.yOffset && offsetY <= this.yOffset + this.unitCellHeight)
            return true;
        return false;
    }

    getTitleLength(withMargin: boolean = true, fontSize = this.cellFontSize): number {

        let iconWidth = 0;
        if (this.icons) {
            for (let icon of this.icons) {
                iconWidth += Math.max(icon.clientWidth, this.iconWidth)
            }
        }

        this.ctx.font = fontSize + 'px serif';
        let returnLength = this.ctx.measureText(this.trackName).width * 1.1
        if (withMargin)
            returnLength += 20
        return returnLength + iconWidth;
    }

    getSeqId(): number {
        return this.sequenceId
    }

    splitSelectedCell() {
        if (this.isValidCellId(this.clickedCell)) {
            this.cellManager.SplitCell(this.clickedCell)
        } else {
            Logger.warn("Invalid cell selection.")
        }
    }

    mergeCells(startCellId: number, endCellId: number) {
        if (!this.isValidCellId(startCellId) || !this.isValidCellId(endCellId)) {
            Logger.error("Trying to merge invalid cells")
            return;
        }

        let startSpanHead = this.cellManager.GetSpanHead(startCellId)
        let endSpanHead = this.cellManager.GetSpanHead(endCellId)

        if (startSpanHead === endSpanHead) {
            return;
        }

        this.cellManager.MergeCells(startSpanHead, endSpanHead);
    }

    mergeSelectedCells() {
        this.mergeCells(this.selectedCellStart, this.selectedCellEnd)
    }

    isValidCellId(cellId: number): boolean {
        if (cellId < 0 || cellId > this.frameCount)
            return false
        return true;
    }

    calculateCellIdx(absoluteOffsetX: number) {
        return Math.floor(absoluteOffsetX / this.unitCellWidth)
    }

    calculateCanvasOffsetX(cellId: number, relative: boolean = true) {
        let absoluteOffsetX = cellId * this.unitCellWidth

        if (!relative)
            return absoluteOffsetX

        let relativeOffsetX = absoluteOffsetX - this.canvasStartPos;

        return relativeOffsetX;
    }

    drawCell(cellId: number, forceDraw: boolean = false) {
        if (!this.isValidCellId(cellId)) { // Invalid cell or the layer is still initing.
            return;
        }

        let inputCellId = cellId;

        if (forceDraw || this.cellManager.IsSpanHead(cellId)) {
            cellId = this.cellManager.GetSpanHead(cellId)

            let spanCellCount = this.cellManager.GetCellSpan(cellId);
            let cellWidth = spanCellCount * this.unitCellWidth;

            let minSelectedCellId = Math.min(this.selectedCellStart, this.selectedCellEnd)
            let maxSelectedCellId = Math.max(this.selectedCellStart, this.selectedCellEnd)

            // Draw the cell
            this.ctx.beginPath()
            if (minSelectedCellId <= cellId && maxSelectedCellId >= cellId) {
                this.ctx.fillStyle = this.selectedCellBgStyle
            } else {
                this.ctx.fillStyle = this.cellBgStyle
            }

            this.ctx.strokeStyle = this.cellStrokeStyle
            let startOffsetX = this.calculateCanvasOffsetX(cellId)
            let startOffsetY = this.yOffset

            this.ctx.fillRect(startOffsetX, startOffsetY, cellWidth, this.unitCellHeight)
            this.ctx.strokeRect(startOffsetX, startOffsetY, cellWidth, this.unitCellHeight)

            if (this.sequenceId == 0 && (cellId == 0 || (cellId + 1) % 5 == 0)) {
                this.ctx.fillStyle = this.cellFontStyle
                this.ctx.font = this.cellFontSize + 'px serif';

                let cellIdStr = (cellId + 1).toString()
                let textMetrics: TextMetrics = this.ctx.measureText(cellIdStr)
                this.ctx.fillText(cellIdStr, startOffsetX + cellWidth / 2 - textMetrics.width / 2, startOffsetY + this.cellFontSize)
            }
        }

        // Draw stop frame
        if (this.layer && this.layer.IsStopFrame(inputCellId)) {
            let inputCellOffsetX = this.calculateCanvasOffsetX(inputCellId)
            this.ctx.fillStyle = this.stopFrameStyle
            this.ctx.fillRect(inputCellOffsetX, this.yOffset, this.unitCellWidth, this.unitCellHeight)
        }

        let keyframeCircleRadius = this.unitCellWidth / 2 * 0.5

        // Draw key frame indicator
        if (this.layer && this.layer.IsKeyFrame(inputCellId)) {
            let inputCellOffsetX = this.calculateCanvasOffsetX(inputCellId)
            this.ctx.beginPath()
            this.ctx.arc(inputCellOffsetX + this.unitCellWidth / 2, this.yOffset + keyframeCircleRadius, keyframeCircleRadius, 0, 2 * Math.PI)
            this.ctx.fillStyle = "black"
            this.ctx.fill()
            this.ctx.strokeStyle = "black"
            this.ctx.stroke()
        }

        // Draw event graph indicator
        if (this.layer && this.layer.HasEventGraph(inputCellId)) {
            let inputCellOffsetX = this.calculateCanvasOffsetX(inputCellId)
            this.ctx.fillStyle = "black"
            this.ctx.font = this.unitCellWidth + "px Arial"
            this.ctx.fillText("G", inputCellOffsetX, this.yOffset + this.unitCellHeight - 2); // Minus 2 pixels as margin.
        }


    }

    addOnMouseMoveFunc(xMin, xMax, onmouseMoveFunc) {
        for (let funcTuple of this.iconMouseMoveFuncArray) {
            if (funcTuple[2] == onmouseMoveFunc)
                return;
        }

        this.iconMouseMoveFuncArray.push([xMin, xMax, onmouseMoveFunc])
    }

    addOnClickFunc(xMin, xMax, onclickFunc) {
        for (let funcTuple of this.iconClickFuncArray) {
            if (funcTuple[2] == onclickFunc)
                return;
        }

        this.iconClickFuncArray.push([xMin, xMax, onclickFunc])
    }

    onNameClicked() {
        if (this.onNameClickedCallback) {
            this.onNameClickedCallback(this.layer)
        }
    }

    bindedOnNameClicked: Function = null
    nameMouseEnter: Function = () => {
        this.ctx.canvas.style.cursor = "pointer"
    }

    drawTrack(canvasStartPos: number, canvasEndPos: number, maxCellId: number = -1) {
        this.canvasStartPos = canvasStartPos
        this.canvasEndPos = canvasEndPos

        let imgXOffset = this.getTitleLength(false, this.trackNameSize)
        let imgYOffset = this.yOffset + (this.unitCellHeight - this.iconHeight) * 0.5

        let xOffset = 0;
        if (this.icons) {
            for (let icon of this.icons) {
                let xMin = xOffset
                this.ctx.drawImage(icon, xOffset, imgYOffset, this.iconWidth, this.iconHeight)
                xOffset += Math.max(icon.clientWidth, this.iconWidth)

                this.addOnClickFunc(xMin, xOffset, icon["onIconClicked"])
            }
        }

        this.ctx.fillStyle = "black"
        this.ctx.font = this.trackNameSize + 'px serif'
        let textYOffset = this.yOffset + this.trackNameSize + (this.unitCellHeight - this.trackNameSize) * 0.5

        this.ctx.fillText(this.trackName, xOffset, textYOffset)

        if (this.bindedOnNameClicked == null) {
            this.bindedOnNameClicked = this.onNameClicked.bind(this)
        }

        this.addOnMouseMoveFunc(xOffset, imgXOffset, this.nameMouseEnter)
        this.addOnClickFunc(xOffset, imgXOffset, this.bindedOnNameClicked)

        if (this.isSelected) {

            let _this = this
            if (this.checkBoxImg.complete) {
                this.ctx.drawImage(this.checkBoxImg, imgXOffset, imgYOffset, this.iconWidth, this.iconHeight)
            } else {
                this.checkBoxImg.onload = (e: Event) => {
                    _this.ctx.drawImage(_this.checkBoxImg, imgXOffset, imgYOffset)
                }
            }
        }

        if (this.cellManager == null) {
            let _this = this
            this.pendingFuncs.push(function () {
                _this.drawTrackInternal(maxCellId)
            })
        } else {
            this.drawTrackInternal(maxCellId)
        }
    }

    drawTrackInternal(maxCellId: number = -1) {
        let startCellIdx = this.calculateCellIdx(this.canvasStartPos - this.unitCellWidth) // Leave one cell margin
        let endCellIdx = this.calculateCellIdx(this.canvasEndPos + this.unitCellWidth)

        let firstCellX = this.calculateCanvasOffsetX(0);
        this.ctx.strokeStyle = "black"
        this.ctx.strokeRect(0, this.yOffset, firstCellX, this.getCellHeight())

        for (let cellIdx = startCellIdx; cellIdx <= endCellIdx; cellIdx++) {
            if (cellIdx == startCellIdx) {
                this.drawCell(cellIdx, true);
            } else {
                this.drawCell(cellIdx, false);
            }
        }

        this.drawTimelineIndicator()

        if (maxCellId >= 0) {
            // Draw the red time line indicator
            let offsetX = this.calculateCanvasOffsetX(maxCellId)

            let oldLineWidth = this.ctx.lineWidth
            this.ctx.beginPath();
            this.ctx.strokeStyle = "black"
            this.ctx.lineWidth = 10;
            this.ctx.moveTo(offsetX, this.yOffset);
            this.ctx.lineTo(offsetX, this.yOffset + this.getCellHeight())
            this.ctx.stroke()
            this.ctx.lineWidth = oldLineWidth
        }
    }

    getCurrentCellId() {
        return Math.floor(this.elapsedTime * GlobalConfig.fps)
    }

    drawTimelineIndicator() {
        // Draw the red time line indicator
        let offsetX = this.calculateCanvasOffsetX(this.elapsedTime * GlobalConfig.fps)
        this.ctx.beginPath();
        this.ctx.strokeStyle = "red"
        this.ctx.moveTo(offsetX, this.yOffset);
        this.ctx.lineTo(offsetX, this.yOffset + this.getCellHeight())
        this.ctx.stroke()
    }

    unSelectTrack() {
        this.isSelected = false
    }

    handleMouseMove(relativeX: number) {
        let handled = false
        for (let funcTuple of this.iconMouseMoveFuncArray) {
            let min = funcTuple[0]
            let max = funcTuple[1]
            if (relativeX > min && relativeX < max) {
                if (funcTuple[2]) {
                    funcTuple[2](this.layer)
                    handled = true
                }
            }
        }

        if (!handled) {
            this.onMouseLeave()
        }
    }

    onMouseMove(relativeX: number) {
        this.handleMouseMove(relativeX)
    }

    onMouseEnter(relativeX: number) {
        this.handleMouseMove(relativeX)
    }

    onMouseLeave() {
        this.ctx.canvas.style.cursor = "default"
    }

    clickedTrack(relativeX: number | null, resetRangeSelect: boolean) {
        if (!this.selectable)
            return;

        for (let funcTuple of this.iconClickFuncArray) {
            let min = funcTuple[0]
            let max = funcTuple[1]
            if (relativeX > min && relativeX < max) {
                if (funcTuple[2])
                    funcTuple[2](this.layer)
            }
        }

        this.isSelected = true;

        if (relativeX != null) {
            let absoluteX = this.canvasStartPos + relativeX;
            let cellId = this.calculateCellIdx(absoluteX);

            this.selectCell(cellId, resetRangeSelect)
        }

        if (this.layer) {
            let store = this.layer.GetObjectStore()
            if (store != null)
                store.SetCurrentLayer(this.layer)
        }

    }

    clickedCell = -1

    selectCell(cellId, resetRangeSelect) {
        if (this.isValidCellId(cellId)) {
            this.clickedCell = cellId

            if (resetRangeSelect) {
                let spanHeadCellId = this.cellManager.GetSpanHead(cellId)

                this.selectedCellStart = spanHeadCellId;
                this.selectedCellEnd = spanHeadCellId;
            }

            this.emit(TimelineTrackEventNames.CELLCLICKED, this, cellId)
        }
    }

    rangeSelect(relativeX: number) {
        if (!this.selectable)
            return;

        let absoluteX = this.canvasStartPos + relativeX;
        let cellId = this.calculateCellIdx(absoluteX)

        this.selectedCellEnd = this.cellManager.GetSpanHead(cellId);
    }

    clearSelect() {
        if (!this.selectable)
            return;

        this.selectedCellStart = -1;
        this.selectedCellEnd = -1;
    }
}

class DefaultCellManager {
    GetSpanHead(cellId) {
        return cellId
    }

    GetCellSpan(cellId) {
        return 1;
    }

    IsSpanHead(cellId) {
        return true;
    }
}

class TitleTimelineTrack extends TimelineTrack {
    public IgnoreDuringLoad() {
        return true
    }

    constructor(sequenceId: number, frameCount: number, ctx: CanvasRenderingContext2D, yOffset = 0, layer = null, trackName: string = "NoNameTrack") {
        super(sequenceId, frameCount, ctx, yOffset, layer, trackName);

        this.unitCellHeight = this.cellFontSize * 1.3
        this.selectable = false

        this.cellBgStyle = "silver"

        this.cellManager = new DefaultCellManager()
    }
}

export {TimelineTrack, TitleTimelineTrack, TimelineTrackEventNames, ICONWIDTH, ICONHEIGHT}