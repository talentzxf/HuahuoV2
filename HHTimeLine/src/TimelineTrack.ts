import {TypedEmitter} from 'tiny-typed-emitter';

enum TimelineTrackEventNames {
    CELLSELECTED = 'cellSelected'
}

interface TimelineTrackEvent {
    'cellSelected': (cellId: number) => void;
}

class CellManager{
    // Multiple cells might be merged into one big cell, so record it in the cellWidthMap
    cellWidthMap: Map<number, number> = new Map();

    // Map from the cellId to it's begining cell.
    mergedCells: Map<number, number> = new Map();

    isMerged(cellId){
        return false;
    }

    getCellSpan(cellId){
        return 1;
    }
}

class TimelineTrack extends TypedEmitter<TimelineTrackEvent> {
    static unitCellWidth: number = 20;
    static unitCellHeight: number = 30;

    ctx: CanvasRenderingContext2D
    frameCount: number;
    canvas: HTMLCanvasElement
    sequenceId: number

    cellManager: CellManager = new CellManager()

    selectedCellStart: number = -1;
    selectedCellEnd: number = -1;

    cellBgStyle: string = "lightgray"
    selectedCellBgStyle: string = "cyan"
    cellStrokeStyle: string = "black"
    cellFontSize: number = 10
    cellFontStyle: string = "black"

    trackeName: string;
    canvasStartPos: number;
    canvasEndPos: number;

    constructor(sequenceId: number, frameCount: number, ctx: CanvasRenderingContext2D, trackName: string = "NoNameTrack") {
        super();
        this.sequenceId = sequenceId;
        this.trackeName = trackName;
        this.frameCount = frameCount;
        this.ctx = ctx
    }

    mergeSelectedCells(){
    }

    isValidCellId(cellId: number): boolean {
        if (cellId < 0 || cellId > this.frameCount)
            return false
        return true;
    }

    calculateCellIdx(absoluteOffsetX: number) {
        return Math.floor(absoluteOffsetX / TimelineTrack.unitCellWidth)
    }

    calculateCanvasOffsetX(cellId: number) {
        let absoluteOffsetX = cellId * TimelineTrack.unitCellWidth
        let relativeOffsetX = absoluteOffsetX - this.canvasStartPos;

        return relativeOffsetX;
    }

    calculateCanvasOffsetY(cellId: number) {
        return this.sequenceId * TimelineTrack.unitCellHeight
    }

    drawCell(cellId: number) {
        if (!this.isValidCellId(cellId)) {
            return;
        }

        if(this.cellManager.isMerged(cellId)){ // Won't draw merged cells
            return
        }

        let spanCellCount = this.cellManager.getCellSpan(cellId);
        let cellWidth = spanCellCount * TimelineTrack.unitCellWidth;

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
        let startOffsetY = this.calculateCanvasOffsetY(cellId)

        this.ctx.fillRect(startOffsetX, startOffsetY, cellWidth, TimelineTrack.unitCellHeight)
        this.ctx.strokeRect(startOffsetX, startOffsetY, cellWidth, TimelineTrack.unitCellHeight)

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

        let startCellIdx = this.calculateCellIdx(this.canvasStartPos - TimelineTrack.unitCellWidth) // Leave one cell margin
        let endCellIdx = this.calculateCellIdx(this.canvasEndPos + TimelineTrack.unitCellWidth)

        for (let cellIdx = startCellIdx; cellIdx <= endCellIdx; cellIdx++) {
            this.drawCell(cellIdx);
        }
    }

    onTrackClick(relativeX: number) {
        let absoluteX = this.canvasStartPos + relativeX;
        let cellId = this.calculateCellIdx(absoluteX);

        if (this.cellManager.isMerged(cellId)) {
            console.log("TODO:  Merge cells !!!!")
        } else {
            this.selectedCellStart = cellId
            this.selectedCellEnd = Math.max( cellId, this.selectedCellEnd);
        }
    }

    rangeSelect(relativeX: number){
        let absoluteX = this.canvasStartPos + relativeX;
        let cellId = this.calculateCellIdx(absoluteX)

        this.selectedCellEnd = cellId;
    }

    clearSelect() {
        this.selectedCellStart = -1;
        this.selectedCellEnd = -1;
    }
}

export {TimelineTrack}