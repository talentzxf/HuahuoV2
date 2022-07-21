import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {paper, BaseShapeJS} from "hhenginejs"
import {Vector2} from "hhcommoncomponents"

class ShapeRotateHandler extends ShapeTranslateMorphBase{
    protected rotationCenter:paper.Point
    protected targetShape: BaseShapeJS
    protected lastPos: paper.Point = null

    private rotationDegree: number = 0.0;

    constructor() {
        super();
    }

    beginMove(startPos){
        super.beginMove(startPos);
        this.lastPos = new paper.Point(startPos.x, startPos.y)
        this.targetShape = this.curObjs.values().next().value // There's only one object in the set, get it.

        this.rotationCenter = this.targetShape.position

        this.rotationDegree = 0.0;
    }

    dragging(pos) {
        super.dragging(pos);

        if(this.isDragging && this.targetShape != null){
            let vec1 = this.lastPos.subtract(this.rotationCenter)
            let vec2 = pos.subtract(this.rotationCenter)

            let theta = vec1.getDirectedAngle(vec2)

            this.rotationDegree += theta

            console.log("Rotation degree is:" + this.rotationDegree)

            this.targetShape.rotate(theta, this.rotationCenter)
            this.lastPos = new paper.Point(pos.x, pos.y)
            this.targetShape.store()
            this.targetShape.updateBoundingBox()
        }
    }
}

let shapeRotateHandler = new ShapeRotateHandler()
export {shapeRotateHandler}