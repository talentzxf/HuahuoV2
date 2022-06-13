import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2} from "hhcommoncomponents"
import {paper} from "hhenginejs";

class ShapeSelector extends BaseShapeDrawer{
    selectRectangle: paper.Path.Rectangle;
    imgClass = "fas fa-arrow-pointer"
    name = "ShapeSelector"

    startPos: Vector2;

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
    }

    onMouseDown(evt: MouseEvent) {
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

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        this.isDrawing = false
        this.selectRectangle.remove()
    }
}

export {ShapeSelector}