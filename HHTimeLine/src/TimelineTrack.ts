import {TypedEmitter} from 'tiny-typed-emitter';
import {GlobalConfig} from "./GlobalConfig";
import {Logger} from "hhcommoncomponents";

enum TimelineTrackEventNames {
    CELLCLICKED = 'cellClicked'
}

declare class TimeLineTrack {}

interface TimelineTrackEvent {
    'cellClicked': (track:TimeLineTrack, cellId: number) => void;
}

class CellManager{
    // Multiple cells might be merged into one big cell, so record how many cells are there in the cellSpanMap
    cellSpanMap: Map<number, number> = new Map();

    // Map from the cellId to it's begining cell.
    mergedCells: Map<number, number> = new Map();

    getSpanHead(cellId){
        if(!this.mergedCells.has(cellId)){
            return cellId
        }

        return this.mergedCells.get(cellId)
    }

    isSpanHead(cellId){
        if(!this.mergedCells.has(cellId) || this.mergedCells.get(cellId) == cellId){
            return true
        }

        return false
    }

    getCellSpan(cellId){
        if(!this.cellSpanMap.has(cellId))
            return 1;
        return this.cellSpanMap.get(cellId)
    }

    mergeCells(selectedStart, selectedEnd){
        let minCell = Math.min(selectedStart, selectedEnd)
        let maxCell = Math.max(selectedStart, selectedEnd)
        let currentMaxCellSpan = this.getCellSpan(maxCell)

        let newMinCellSpan = maxCell - minCell + currentMaxCellSpan;
        // Update all spans in the middle
        for(let cellId = minCell; cellId <= minCell + newMinCellSpan - 1; cellId++){
            // 1. Delete all cell spans in the middle
            if(this.cellSpanMap.get(cellId)){
                this.cellSpanMap.delete(cellId)
            }
            this.mergedCells.set(cellId, minCell)
        }

        this.cellSpanMap.set(minCell, newMinCellSpan)
    }
}

class TimelineTrack extends TypedEmitter<TimelineTrackEvent> {
    static defaultUnitCellWidth: number = 20;
    static defaultUnitCellHeight: number = 30;

    protected unitCellWidth: number = -1;
    protected unitCellHeight: number = -1;
    protected selectable: boolean = true;
    protected cellFontSize: number = 10
    protected cellBgStyle: string = "lightgray"

    private ctx: CanvasRenderingContext2D
    private frameCount: number;
    private sequenceId: number

    private cellManager: CellManager = new CellManager()

    private selectedCellStart: number = -1;
    private selectedCellEnd: number = -1;


    private selectedCellBgStyle: string = "cyan"
    private cellStrokeStyle: string = "black"
    private cellFontStyle: string = "black"

    private trackeName: string;
    private canvasStartPos: number;
    private canvasEndPos: number;

    private yOffset: number;

    private elapsedTime: number = 0.0; // In seconds.

    constructor(sequenceId: number, frameCount: number, ctx: CanvasRenderingContext2D, yOffset = 0, trackName: string = "NoNameTrack") {
        super();
        this.sequenceId = sequenceId;
        this.trackeName = trackName;
        this.frameCount = frameCount;
        this.ctx = ctx

        this.unitCellWidth = TimelineTrack.defaultUnitCellWidth
        this.unitCellHeight = TimelineTrack.defaultUnitCellHeight
        this.yOffset = yOffset
    }

    setElapsedTime(elapsedTime){
        this.elapsedTime = elapsedTime
    }

    getYOffset():number{
        return this.yOffset;
    }

    getCellHeight():number{
        return this.unitCellHeight
    }

    getCellWidth():number{
        return this.unitCellWidth
    }

    hasYOffset(offsetY: number):boolean{
        if(offsetY >= this.yOffset && offsetY <= this.yOffset + this.unitCellHeight)
            return true;
        return false;
    }

    getTitleLength():number{
        this.ctx.font = this.cellFontSize + 'px serif';
        return this.ctx.measureText(this.trackeName).width * 1.1
    }

    getSeqId():number{
        return this.sequenceId
    }

    mergeSelectedCells(){
        if(!this.isValidCellId(this.selectedCellStart) || !this.isValidCellId(this.selectedCellEnd)){
            Logger.error("Trying to merge invalid cells")
            return;
        }

        let startSpanHead = this.cellManager.getSpanHead(this.selectedCellStart)
        let endSpanHead = this.cellManager.getSpanHead(this.selectedCellEnd)

        if(startSpanHead === endSpanHead){
            return;
        }

        this.cellManager.mergeCells(startSpanHead, endSpanHead);
    }

    isValidCellId(cellId: number): boolean {
        if (cellId < 0 || cellId > this.frameCount)
            return false
        return true;
    }

    calculateCellIdx(absoluteOffsetX: number) {
        return Math.floor(absoluteOffsetX / this.unitCellWidth)
    }

    calculateCanvasOffsetX(cellId: number) {
        let absoluteOffsetX = cellId * this.unitCellWidth
        let relativeOffsetX = absoluteOffsetX - this.canvasStartPos;

        return relativeOffsetX;
    }

    drawCell(cellId: number) {
        if (!this.isValidCellId(cellId)) {
            return;
        }

        // VZ: For merged cells, always draw it's span head. This will cause the cell be redrawn many times during redraw. But might not be a big deal ??
        cellId = this.cellManager.getSpanHead(cellId)

        let spanCellCount = this.cellManager.getCellSpan(cellId);
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

    drawTrack(canvasStartPos: number, canvasEndPos: number) {
        this.canvasStartPos = canvasStartPos
        this.canvasEndPos = canvasEndPos

        let startCellIdx = this.calculateCellIdx(this.canvasStartPos - this.unitCellWidth) // Leave one cell margin
        let endCellIdx = this.calculateCellIdx(this.canvasEndPos + this.unitCellWidth)

        this.ctx.fillStyle = "black"
        this.ctx.font = this.cellFontSize + 'px serif'
        this.ctx.fillText(this.trackeName, 0, this.yOffset + this.cellFontSize)

        let firstCellX = this.calculateCanvasOffsetX(0);
        this.ctx.strokeStyle = "black"
        this.ctx.strokeRect(0, this.yOffset, firstCellX, this.getCellHeight())

        for (let cellIdx = startCellIdx; cellIdx <= endCellIdx; cellIdx++) {
            this.drawCell(cellIdx);
        }

        // Draw the red time line indicator
        let offsetX = this.calculateCanvasOffsetX(this.elapsedTime * GlobalConfig.fps)
        this.ctx.beginPath();
        this.ctx.strokeStyle = "red"
        this.ctx.moveTo(offsetX, this.yOffset);
        this.ctx.lineTo(offsetX, this.yOffset + this.getCellHeight())
        this.ctx.stroke()
    }

    onTrackClick(relativeX: number) {
        if(!this.selectable)
            return;

        let absoluteX = this.canvasStartPos + relativeX;
        let cellId = this.calculateCellIdx(absoluteX);

        cellId = this.cellManager.getSpanHead(cellId)

        if(this.isValidCellId(cellId)){
            this.selectedCellStart = cellId
            this.selectedCellEnd = cellId;

            this.emit(TimelineTrackEventNames.CELLCLICKED, this, cellId)
        }
    }

    rangeSelect(relativeX: number){
        if(!this.selectable)
            return;

        let absoluteX = this.canvasStartPos + relativeX;
        let cellId = this.calculateCellIdx(absoluteX)

        this.selectedCellEnd = this.cellManager.getSpanHead(cellId);
    }

    clearSelect() {
        if(!this.selectable)
            return;

        this.selectedCellStart = -1;
        this.selectedCellEnd = -1;
    }
}

class TitleTimelineTrack extends TimelineTrack{
    constructor(sequenceId: number, frameCount: number, ctx: CanvasRenderingContext2D, yOffset = 0, trackName: string = "NoNameTrack") {
        super(sequenceId, frameCount, ctx, yOffset, trackName);

        this.unitCellHeight = this.cellFontSize * 1.2
        this.selectable = false

        this.cellBgStyle = "silver"
    }
}

export {TimelineTrack, TitleTimelineTrack, TimelineTrackEventNames}