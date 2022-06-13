import {BaseShapeJS} from "hhenginejs"
import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2} from "hhcommoncomponents"
import {paper} from "hhenginejs";

class ShapeSelector extends BaseShapeDrawer{
    selectRectangle: paper.Path.Rectangle;
    imgClass = "fas fa-arrow-pointer"
    name = "ShapeSelector"

    startPos: Vector2;
    margin: number = 5;
    hitOptions = {}

    selectedShapes: Array<BaseShapeJS> = new Array()

    constructor() {
        super();

        this.hitOptions = {
            segments: true,
            stroke: true,
            fill: true,
            handles: true,
            tolerance: 5
        }
    }

    isDefaultDrawer(): boolean {
        return true
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
    }

    onMouseDown(evt: MouseEvent) {
        // Clear current selections.
        for(let shape of this.selectedShapes){
            shape.selected = false
            shape.update()
        }

        this.selectedShapes = new Array()

        super.onMouseDown(evt);
        this.isDrawing = true;
        this.startPos = this.getWorldPosFromView(evt.offsetX, evt.offsetY)
    }

    onMouseMove(evt: MouseEvent) {
        super.onMouseMove(evt);

        if(this.isDrawing){

            let endPos = this.getWorldPosFromView(evt.offsetX, evt.offsetY)

            if(this.selectRectangle){
                this.selectRectangle.remove()
            }

            this.selectRectangle = new paper.Path.Rectangle(this.startPos, endPos)
            this.selectRectangle.strokeColor = new paper.Color("Black")
            this.selectRectangle.dashArray = [10, 12];
        }
    }

    itemSelectable(item){
        if( typeof item.data.meta == "undefined" || false == item.data.meta.isSelectable())
            return false
        return true
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        if(this.isDrawing){
            let endPos = this.getWorldPosFromView(evt.offsetX, evt.offsetY)
            if(endPos.distance(this.startPos) < this.margin){
                let screenPos = new paper.Point(evt.offsetX, evt.offsetY)

                // Single click, perform hit test.
                let hitResult = paper.project.hitTest(screenPos, this.hitOptions)
                if(hitResult){
                    let hitItem = hitResult.item;
                    if(this.itemSelectable(hitResult.item)){
                        hitItem.data.meta.selected = true
                        hitItem.data.meta.update();

                        this.selectedShapes.push(hitItem.data.meta)
                    }
                }
            }
        }

        this.isDrawing = false

        if(this.selectRectangle)
            this.selectRectangle.remove()
    }
}

export {ShapeSelector}