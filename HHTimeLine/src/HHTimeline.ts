import {CustomElement} from "./CustomComponent";

class TimelineTrack {
    static unitCellWidth: number = 20;
    static unitCellHeight: number = 30;

    ctx: CanvasRenderingContext2D
    frameCount: number;
    canvas: HTMLCanvasElement
    sequenceId: number
    // Multiple cells might be merged into one big cell, so record it in the cellWidthMap
    cellWidthMap: Map<number, number> = new Map();
    mergedCells: Set<number> = new Set();
    selectedCells: Set<number> = new Set();

    cellBgStyle: string = "lightgray"
    selectedCellBgStyle:string = "blue"
    cellStrokeStyle: string = "black"
    cellFontSize: number = 10
    cellFontStyle: string = "black"

    trackeName: string;
    canvasStartPos: number;
    canvasEndPos: number;

    constructor(sequenceId: number, frameCount: number, ctx: CanvasRenderingContext2D, trackName: string = "NoNameTrack") {
        this.sequenceId = sequenceId;
        this.trackeName = trackName;
        this.frameCount = frameCount;
        this.ctx = ctx
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

    calculateCanvasOffsetY(cellId: number){
        return this.sequenceId * TimelineTrack.unitCellHeight
    }

    drawCell(cellId: number) {
        if (!this.isValidCellId(cellId)) {
            return;
        }
        if (this.mergedCells.has(cellId)) // Won's draw merged cells.
            return;

        let cellWidth = TimelineTrack.unitCellWidth;
        if (this.cellWidthMap.has(cellId)) {
            cellWidth = this.cellWidthMap.get(cellId);
        }

        // Draw the cell
        this.ctx.beginPath()
        if(this.selectedCells.has(cellId)){
            this.ctx.fillStyle = this.selectedCellBgStyle
        }else{
            this.ctx.fillStyle = this.cellBgStyle
        }

        this.ctx.strokeStyle = this.cellStrokeStyle
        let startOffsetX = this.calculateCanvasOffsetX(cellId)
        let startOffsetY = this.calculateCanvasOffsetY(cellId)

        this.ctx.fillRect(startOffsetX, startOffsetY, cellWidth, TimelineTrack.unitCellHeight)
        this.ctx.strokeRect(startOffsetX, startOffsetY, cellWidth, TimelineTrack.unitCellHeight)

        if (cellId == 0 || (cellId + 1) % 5 == 0) {
            this.ctx.fillStyle = this.cellFontStyle
            this.ctx.font = this.cellFontSize + 'px serif';

            let cellIdStr = (cellId + 1).toString()
            let textMetrics: TextMetrics = this.ctx.measureText(cellIdStr)
            this.ctx.fillText(cellIdStr, startOffsetX + cellWidth / 2 - textMetrics.width/2, startOffsetY + this.cellFontSize)
        }
    }

    drawTrack(canvasStartPos:number, canvasEndPos:number){
        this.canvasStartPos = canvasStartPos
        this.canvasEndPos = canvasEndPos

        let startCellIdx = this.calculateCellIdx(this.canvasStartPos - TimelineTrack.unitCellWidth) // Leave one cell margin
        let endCellIdx = this.calculateCellIdx(this.canvasEndPos + TimelineTrack.unitCellWidth)

        for (let cellIdx = startCellIdx; cellIdx <= endCellIdx; cellIdx++) {
            this.drawCell(cellIdx);
        }
    }

    onTrackClick(relativeX: number){
        let absoluteX = this.canvasStartPos + relativeX;
        let cellId = this.calculateCellIdx(absoluteX);

        if(this.mergedCells.has(cellId)){
            console.log("TODO:  Merge cells !!!!")
        }else{
            this.selectedCells.add(cellId)
        }
    }
}

// TODO: Multiple tracks????
@CustomElement({
    "selector": "hh-timeline"
})
class HHTimeline extends HTMLElement {
    frameCount: number = 1000000
    canvasScrollContainer: HTMLDivElement = null // This will show the scrollbar.
    canvasContainer: HTMLDivElement = null; // This will contain the canvas

    canvas: HTMLCanvasElement = null
    canvasStartPos: number = 0
    canvasEndPos: number = -1
    ctx: CanvasRenderingContext2D = null;
    canvasWidth: number = -1;
    canvasHeight: number = -1;

    timelineTracks: Array<TimelineTrack> = new Array()

    connectedCallback() {
        // canvasScrollContainer wraps canvasContainer wraps cavnas.
        // Because canvas has a width limit. So the canvas can't be very big: https://www.tutorialspoint.com/Maximum-size-of-a-canvas-element-in-HTML#:~:text=All%20web%20browsers%20limit%20the,allowable%20area%20is%20268%2C435%2C456%20pixels.

        this.canvasScrollContainer = document.createElement("div")
        this.canvasContainer = document.createElement("div")
        this.canvas = document.createElement("canvas");
        this.ctx = this.canvas.getContext("2d");
        this.appendChild(this.canvasScrollContainer)
        this.canvasScrollContainer.appendChild(this.canvasContainer)
        this.canvasContainer.appendChild(this.canvas)

        this.canvasScrollContainer.style.overflowX = "auto"
        this.canvasScrollContainer.style.overflowY = "hidden"
        this.canvas.style.position = "relative"

        this.canvasScrollContainer.addEventListener("scroll", this.onScroll.bind(this))
        window.addEventListener("resize", this.Resize.bind(this))

        // Add one timelinetrack
        this.timelineTracks.push(new TimelineTrack(0, this.frameCount, this.canvas.getContext('2d')))
        this.timelineTracks.push(new TimelineTrack(1, this.frameCount, this.canvas.getContext('2d')))
        this.timelineTracks.push(new TimelineTrack(2, this.frameCount, this.canvas.getContext('2d')))
        this.canvas.addEventListener("click", this.onCanvasClick.bind(this))

        this.Resize();
    }

    calculateTrackSeqId(offsetY: number){
        return Math.floor( offsetY / TimelineTrack.unitCellHeight );
    }

    onCanvasClick(evt:MouseEvent){
        let trackSeqId = this.calculateTrackSeqId( evt.offsetY )
        if(trackSeqId < 0 || trackSeqId >= this.timelineTracks.length){
            console.log("Error clientY!!!")
            return;
        }

        this.timelineTracks[trackSeqId].onTrackClick(evt.clientX);

        this.redrawCanvas()
    }

    Resize() {
        console.log("On Resize")
        let widthPixel: number = this.frameCount * TimelineTrack.unitCellWidth;

        let trackNumber = this.timelineTracks.length;
        let heightPixel: number = trackNumber * TimelineTrack.unitCellHeight;
        this.canvasContainer.style.width = widthPixel + "px";
        this.canvasContainer.style.height = heightPixel + "px";
        this.canvas.width = this.canvasScrollContainer.clientWidth;
        this.canvas.height = this.canvasScrollContainer.clientHeight;

        this.redrawCanvas();
    }

    onScroll() {
        this.redrawCanvas()
    }

    updateStartEndPos() {
        this.canvasWidth = this.canvasScrollContainer.clientWidth
        this.canvasStartPos = this.canvasScrollContainer.scrollLeft;
        this.canvasEndPos = this.canvasStartPos + this.canvasWidth;

        this.canvas.style.left = this.canvasScrollContainer.scrollLeft + "px";
    }

    redrawCanvas() {
        this.updateStartEndPos()
        let ctx = this.ctx
        ctx.clearRect(0, 0, this.canvasWidth, this.canvasHeight)
        ctx.strokeRect(0, 0, this.canvasWidth, this.canvasHeight)

        for(let track of this.timelineTracks){
            track.drawTrack(this.canvasStartPos, this.canvasEndPos);
        }
    }
}

export {HHTimeline}