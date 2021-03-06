import {TimelineTrack, TimelineTrackEventNames, TitleTimelineTrack} from "./TimelineTrack";
import {ContextMenu, CustomElement, Logger} from "hhcommoncomponents";
import {GlobalConfig} from "hhenginejs";
import {TimelineEventNames} from "./HHTimelineEvents";
import {huahuoEngine} from "hhenginejs"

@CustomElement({
    "selector": "hh-timeline"
})
class HHTimeline extends HTMLElement {
    private frameCount: number = 1000
    private canvasScrollContainer: HTMLDivElement = null // This will show the scrollbar.
    private canvasContainer: HTMLDivElement = null; // This will contain the canvas

    private canvas: HTMLCanvasElement = null
    private canvasStartPos: number = 0
    private canvasEndPos: number = -1
    private ctx: CanvasRenderingContext2D = null;
    private canvasWidth: number = -1;
    private canvasHeight: number = -1;

    private timelineTracks: Array<TimelineTrack> = new Array()

    private selectedTrackSeqId: number = -1;

    private isSelectingRangeCell: boolean = false;

    private contextMenu: ContextMenu = new ContextMenu()

    private titleTrack: TitleTimelineTrack;
    private totalTrackHeight: number;
    private layerTrackMap: Map<object, TimelineTrack> = new Map();
    private layerIconMap: Map<Object, Array<any>> = new Map()

    public elapsedTime: number = 0.0

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

        let titleTimeLineTrack = new TitleTimelineTrack(0, this.frameCount, this.canvas.getContext('2d'), 0, null,"Frames")
        this.titleTrack = titleTimeLineTrack
        // Add one timelinetrack
        this.timelineTracks.push(titleTimeLineTrack)
        this.totalTrackHeight = titleTimeLineTrack.getCellHeight()

        //
        this.canvas.addEventListener('mousedown', this.onMouseDown.bind(this))
        this.canvas.addEventListener('mousemove', this.onMouseMove.bind(this))
        this.canvas.addEventListener('mouseup', this.onMouseUp.bind(this))

        this.canvas.addEventListener('contextmenu', this.contextMenu.onContextMenu.bind(this.contextMenu))
        this.setTimeElapsed(0.5 / GlobalConfig.fps)
    }

    setTimeElapsed(playTime) {
        this.elapsedTime = playTime
        for (let tracks of this.timelineTracks) {
            tracks.setElapsedTime(this.elapsedTime)
        }

        this.redrawCanvas()

        if(this.timelineTracks.length >= 1){
            let indicatorOffsetX = this.timelineTracks[0].calculateCanvasOffsetX(Math.floor(this.elapsedTime * GlobalConfig.fps), false)

            let deltaIndicatorOffsetX = indicatorOffsetX - this.canvasScrollContainer.scrollLeft
            if(deltaIndicatorOffsetX > this.canvasScrollContainer.clientWidth || deltaIndicatorOffsetX < 0){
                this.canvasScrollContainer.scrollLeft += deltaIndicatorOffsetX
            }
        }
    }

    reloadTracks(){
        // if(this.selectedTrackSeqId >= 0){
        //     this.timelineTracks[this.selectedTrackSeqId].clearSelect()
        //     this.timelineTracks[this.selectedTrackSeqId].unSelectTrack()
        //     this.selectedTrackSeqId = -1
        // }

        this.timelineTracks = new Array<TimelineTrack>()
        this.timelineTracks.push(this.titleTrack)
        let currentStore = huahuoEngine.GetCurrentStore()
        let layerCount = currentStore.GetLayerCount()
        this.totalTrackHeight = this.titleTrack.getCellHeight()
        for(let layerId = 0 ; layerId < layerCount; layerId++){
            let layer = currentStore.GetLayer(layerId)

            let icons = null
            if(this.layerIconMap.has(layer)){
                icons = this.layerIconMap.get(layer)
            }
            this.addNewTrack(layer, icons)
        }

        this.redrawCanvas()
    }

    addNewTrack(layer = null, icons: Array<any> = null) {
        let seqId = this.timelineTracks.length;

        let track = new TimelineTrack(seqId, this.frameCount, this.canvas.getContext('2d'), this.totalTrackHeight, layer, "Track " + seqId)
        if(icons && icons.length > 0){
            track.setIcons(icons)
            this.layerIconMap.set(track.getLayer(), icons)
        }

        track.setElapsedTime(this.elapsedTime)
        this.timelineTracks.push(track)
        this.totalTrackHeight += track.getCellHeight()

        this.Resize();
        this.redrawCanvas()

        track.on(TimelineTrackEventNames.CELLCLICKED, this.onCellClicked.bind(this))

        let customEvent = new CustomEvent(TimelineEventNames.NEWTRACKADDED, {
            detail: {
                targetObj: track
            }
        })
        this.dispatchEvent(customEvent)
        return track
    }

    onCellClicked(track, cellId) {
        let elapsedTime = (cellId + 0.5) / GlobalConfig.fps
        this.setTimeElapsed(elapsedTime)

        let customEvent = new CustomEvent(TimelineEventNames.TRACKCELLCLICKED, {
            detail:{
                track: track,
                cellId: cellId,
                elapsedTime: elapsedTime
            }
        })

        this.dispatchEvent(customEvent)
    }

    mergeCells() {
        // TODO: Save this command in the Undo stack...
        if (this.isTrackSeqIdValid(this.selectedTrackSeqId)) {
            this.timelineTracks[this.selectedTrackSeqId].mergeSelectedCells()
            this.redrawCanvas()
        } else {
            Logger.error("Error seqId when trying to merge cells:" + this.selectedTrackSeqId)
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
        for (let track of this.timelineTracks) {
            if (track.hasYOffset(offsetY))
                return track.getSeqId()
        }
    }

    isTrackSeqIdValid(seqId: number): boolean {
        if (seqId < 0 || seqId >= this.timelineTracks.length)
            return false

        return true
    }

    onCanvasClick(evt: MouseEvent) {
        let trackSeqId = this.calculateTrackSeqId(evt.offsetY)
        if (trackSeqId < 0 || trackSeqId >= this.timelineTracks.length) {
            Logger.error("Error clientY!!!")
            return;
        }

        let shiftPressed = evt.shiftKey;
        if(!shiftPressed || this.selectedTrackSeqId < 0){
            if (this.selectedTrackSeqId >= 0) {
                this.timelineTracks[this.selectedTrackSeqId].clearSelect();
            }
            this.timelineTracks[trackSeqId].selectTrack(evt.clientX);

            if(this.selectedTrackSeqId >= 0 && this.selectedTrackSeqId != trackSeqId){
                this.timelineTracks[this.selectedTrackSeqId].unSelectTrack();
            }
            this.selectedTrackSeqId = trackSeqId;
        }else{
            this.timelineTracks[trackSeqId].rangeSelect(evt.clientX);
        }

        this.redrawCanvas()
    }

    Resize() {
        Logger.info("On Resize")
        let widthPixel: number = this.frameCount * this.titleTrack.getCellWidth();

        let heightPixel: number = this.totalTrackHeight;
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
        this.canvasHeight = this.canvasScrollContainer.clientHeight
        this.canvasStartPos = this.canvasScrollContainer.scrollLeft;
        this.canvasEndPos = this.canvasStartPos + this.canvasWidth;

        this.canvas.style.left = this.canvasScrollContainer.scrollLeft + "px";
    }

    getTrackFromLayer(layer):TimelineTrack{
        if(!this.layerTrackMap.has(layer)){
            for(let track of this.timelineTracks){
                this.layerTrackMap.set( track.getLayer(), track);
            }
        }

        return this.layerTrackMap.get(layer)
    }

    redrawCell(layer, frameId){
        let track:TimelineTrack = this.getTrackFromLayer(layer)
        if(track){ // It's possible that we are saving an element belong to a layer that's not belong to current timeline. In that case, we won't update the track.
            track.selectCell(frameId)
            track.drawCell(frameId)
            track.drawTimelineIndicator()
        }
    }

    redrawCanvas() {
        this.updateStartEndPos()

        // Clear bg
        this.ctx.fillStyle = "white"
        this.ctx.fillRect(0, 0, this.canvasWidth, this.canvasHeight)

        let maxTrackNameLength = -1;
        for (let track of this.timelineTracks) {
            maxTrackNameLength = Math.max(maxTrackNameLength, track.getTitleLength())
        }

        for (let track of this.timelineTracks) {
            track.drawTrack(this.canvasStartPos - maxTrackNameLength, this.canvasEndPos);
        }
    }
}

export {HHTimeline}