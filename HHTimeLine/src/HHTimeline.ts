import {CustomElement} from "./CustomComponent";
import {TimelineTrack} from "./TimelineTrack";
import {ContextMenu} from "hhcommoncomponents";

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

    selectedTrackSeqId: number = -1;

    isSelectingRangeCell: boolean = false;

    private contextMenu: ContextMenu = new ContextMenu()

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
        // this.canvas.addEventListener("click", this.onCanvasClick.bind(this))

        this.canvas.addEventListener('mousedown', this.onMouseDown.bind(this))
        this.canvas.addEventListener('mousemove', this.onMouseMove.bind(this))
        this.canvas.addEventListener('mouseup', this.onMouseUp.bind(this))

        this.canvas.addEventListener('contextmenu', this.contextMenu.onContextMenu.bind(this.contextMenu))

        this.contextMenu.setItems([
            {
                itemName: "Merged Cells",
                onclick: this.mergeCells.bind(this)
            }
        ])

        this.Resize();
    }

    mergeCells() {
        // TODO: Save this command in the Undo stack...
        if (this.isTrackSeqIdValid(this.selectedTrackSeqId)) {
            this.timelineTracks[this.selectedTrackSeqId].mergeSelectedCells()
        } else {
            console.log("Error seqId when trying to merge cells:" + this.selectedTrackSeqId)
        }
    }

    onMouseDown(evt: MouseEvent) {
        if (evt.buttons == 1) {
            this.onCanvasClick(evt)

            this.isSelectingRangeCell = true;
        }
    }

    onMouseMove(evt: MouseEvent) {
        if (evt.buttons != 1) {
            this.isSelectingRangeCell = false;
        } else {
            if (this.isSelectingRangeCell) {
                if (this.isTrackSeqIdValid(this.selectedTrackSeqId)) {
                    let trackSeqId = this.calculateTrackSeqId(evt.offsetY)

                    if (trackSeqId == this.selectedTrackSeqId) {
                        this.timelineTracks[this.selectedTrackSeqId].rangeSelect(evt.offsetX)
                    }
                }

                this.redrawCanvas()
            }
        }
    }

    onMouseUp(evt: MouseEvent) {
        this.isSelectingRangeCell = false
    }

    calculateTrackSeqId(offsetY: number): number {
        return Math.floor(offsetY / TimelineTrack.unitCellHeight);
    }

    isTrackSeqIdValid(seqId: number): boolean {
        if (seqId < 0 || seqId >= this.timelineTracks.length)
            return false

        return true
    }

    onCanvasClick(evt: MouseEvent) {
        let trackSeqId = this.calculateTrackSeqId(evt.offsetY)
        if (trackSeqId < 0 || trackSeqId >= this.timelineTracks.length) {
            console.log("Error clientY!!!")
            return;
        }

        if (this.selectedTrackSeqId >= 0) {
            this.timelineTracks[this.selectedTrackSeqId].clearSelect();
        }
        this.timelineTracks[trackSeqId].onTrackClick(evt.clientX);
        this.selectedTrackSeqId = trackSeqId;

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

        for (let track of this.timelineTracks) {
            track.drawTrack(this.canvasStartPos, this.canvasEndPos);
        }
    }
}

export {HHTimeline}