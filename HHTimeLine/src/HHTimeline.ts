import {CustomElement} from "./CustomComponent";

@CustomElement({
        "selector": "hh-timeline"
    })
class HHTimeline extends HTMLElement {
    cellCount:number = 100000
    canvasScrollContainer: HTMLDivElement = null // This will show the scrollbar.
    canvasContainer: HTMLDivElement = null; // This will contain the canvas

    canvas:HTMLCanvasElement = null
    canvasStartPos:number = 0
    canvasEndPos:number = -1

    unitCellWidth:number = 20;
    unitCellHeight:number = 30;

    ctx:CanvasRenderingContext2D = null;
    canvasWidth:number = -1;
    canvasHeight:number = this.unitCellHeight;

    // Multiple cells might be merged into one big cell, so record it in the cellWidthMap
    cellWidthMap:Map<number, number> = new Map();
    mergedCells:Set<number> = new Set();

    connectedCallback(){

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

        this.Resize();

        this.canvasScrollContainer.addEventListener("scroll", this.onScroll.bind(this))
        window.addEventListener("resize", this.Resize.bind(this))
    }

    Resize(){
        console.log("On Resize")
        let widthPixel:number = this.cellCount * this.unitCellWidth;
        let heightPixel:number = this.unitCellHeight;
        this.canvasContainer.style.width = widthPixel + "px";
        this.canvasContainer.style.height = heightPixel + "px";
        this.canvas.width = this.canvasScrollContainer.clientWidth;
        this.canvas.height = this.canvasScrollContainer.clientHeight;

        this.redrawCanvas();
    }

    onScroll(){
        this.redrawCanvas()
    }

    updateStartEndPos(){
        this.canvasWidth = this.canvasContainer.clientWidth
        this.canvasStartPos = this.canvasScrollContainer.scrollLeft;
        this.canvasEndPos = this.canvasStartPos + this.canvasWidth;

        this.canvas.style.left = this.canvasScrollContainer.scrollLeft + "px";
    }

    calculateCellIdx(offset: number){
        return offset / this.unitCellWidth
    }

    calculateCanvasOffsetX(cellId: number){
        let absoluteOffsetX = cellId * this.unitCellWidth
        let relativeOffsetX = absoluteOffsetX - this.canvasStartPos

        return relativeOffsetX;
    }

    drawCell(cellId: number){
        if(this.mergedCells.has(cellId)) // Won's draw merged cells.
            return;

        let cellWidth = this.unitCellWidth;
        if(this.cellWidthMap.has(cellId)){
            cellWidth = this.cellWidthMap.get(cellId);
        }

        // Draw the cell
        this.ctx.beginPath()
        let startOffsetX = this.calculateCanvasOffsetX(cellId)
        this.ctx.clearRect(startOffsetX, 0, cellWidth, this.unitCellHeight)
        this.ctx.strokeRect(startOffsetX, 0, cellWidth, this.unitCellHeight)
    }

    redrawCanvas(){
        this.updateStartEndPos()
        let ctx = this.ctx
        ctx.clearRect(0, 0, this.canvasWidth, this.canvasHeight)
        ctx.strokeRect(0, 0, this.canvasWidth, this.canvasHeight)

        let startCellIdx = this.calculateCellIdx(this.canvasStartPos - this.unitCellWidth ) // Leave one cell margin
        let endCellIdx = this.calculateCellIdx(this.canvasEndPos + this.unitCellWidth)

        for(let cellIdx = startCellIdx; cellIdx <= endCellIdx ; cellIdx++){
            this.drawCell(cellIdx);
        }
    }
}

export {HHTimeline}