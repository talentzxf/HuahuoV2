import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {paper, BaseShapeJS} from "hhenginejs";
import {relaxRectangle} from "hhcommoncomponents";

const BOUNDMARGIN = 15

class ShapePicker extends BaseShapeDrawer{
    hitOptions = {}

    boundingBox: paper.Path

    candidateShape: BaseShapeJS

    onShapePicked: Function

    constructor() {
        super();

        this.hitOptions = {
            segments: false,
            stroke: true,
            fill: true,
            handles: false,
            tolerance: 5
        }
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        canvas.style.cursor = "pointer"
    }

    itemSelectable(item) {
        if (typeof item.data.meta == "undefined" || false == item.data.meta.isSelectable())
            return false
        return true
    }

    // Find the outmost parent of the shape (Will change to find the correct hierarchy later
    findParentOf(shape: BaseShapeJS) {
        let parent = shape.getParent()
        let itr = shape

        while (parent != null) {
            itr = parent
            parent = parent.getParent()
        }

        return itr
    }

    refreshBoundingBox(boundingBoxRect:paper.Rectangle){
        this.clearBoundingBox()

        this.boundingBox = new paper.Path.Rectangle(boundingBoxRect)

        this.boundingBox.strokeColor = new paper.Color("blue")
        this.boundingBox.strokeWidth = 5
    }

    hitSomething(scrX, scrY):boolean{
        let hitPoint = BaseShapeDrawer.getWorldPosFromView(scrX, scrY)

        let hitResult = paper.project.hitTest(hitPoint, this.hitOptions)
        if(hitResult){
            let hitItem = hitResult.item
            if(this.itemSelectable(hitResult.item)){
                console.log("HitType:" + hitResult.type)
                this.candidateShape = this.findParentOf( hitItem.data.meta )

                let shapeBounds = relaxRectangle( this.candidateShape.bounds, BOUNDMARGIN)
                this.refreshBoundingBox(shapeBounds)
                return true;
            }
        }

        return false;
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);
        if(!this.hitSomething(evt.offsetX, evt.offsetY)){
            this.clearBoundingBox()
        }
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);
    }

    clearBoundingBox(){
        if(this.boundingBox){
            this.boundingBox.remove()
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);
        this.clearBoundingBox()

        if(this.candidateShape && this.onShapePicked){
            this.onShapePicked(this.candidateShape)
        }
    }
}

export {ShapePicker}