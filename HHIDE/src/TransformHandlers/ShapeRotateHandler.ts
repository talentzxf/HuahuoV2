import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {paper, BaseShapeJS} from "hhenginejs"
import {ShapeMoveCommand} from "../RedoUndo/ShapeMoveCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {ShapeRotateCommand} from "../RedoUndo/ShapeRotateCommand";

class ShapeRotateHandler extends ShapeTranslateMorphBase {
    protected targetShape: BaseShapeJS
    protected lastPos: paper.Point = null

    private lastRotationDegree: number = 0.0
    private rotationDegree: number = 0.0;

    private rotationIndicator: Array<paper.Path> = new Array<paper.Path>()

    protected pressingShift: boolean = false
    private gapDegree = 10

    constructor() {
        super();

        document.body.addEventListener("keydown", this.onKeyDown.bind(this))
        document.body.addEventListener("keyup", this.onKeyUp.bind(this))
    }

    onKeyDown(e:KeyboardEvent){
        if(e.shiftKey)
            this.pressingShift = true
    }

    onKeyUp(e:KeyboardEvent){
        if(!e.shiftKey)
            this.pressingShift = false
    }

    beginMove(startPos) {
        super.beginMove(startPos);
        this.lastPos = new paper.Point(startPos.x, startPos.y)
        this.targetShape = this.curObjs.values().next().value // There's only one object in the set, get it.

        this.rotationDegree = 0.0;
        this.lastRotationDegree = 0.0
    }

    clearRotationIndicator() {
        for (let shape of this.rotationIndicator) {
            shape.remove()
        }

        this.rotationIndicator = new Array<paper.Path>()
    }

    drawFanShape(position: paper.Point, radius: number, startAngle: number, endAngle: number) {
        if (startAngle == 0 && endAngle == 360) {
            let circleShape = paper.Path.Circle(position, radius)
            return circleShape
        }

        let startAngleRad = startAngle / 180.0 * Math.PI
        let endAngleRad = endAngle / 180.0 * Math.PI

        let midAngleRad = 0.5 * (endAngleRad - startAngleRad) + startAngleRad
        let startPoint = new paper.Point(position.x + radius * Math.cos(startAngleRad), position.y + radius * Math.sin(startAngleRad))
        let midPoint = new paper.Point(position.x + radius * Math.cos(midAngleRad), position.y + radius * Math.sin(midAngleRad))
        let endPoint = new paper.Point(position.x + radius * Math.cos(endAngleRad), position.y + radius * Math.sin(endAngleRad))

        let fanShape = paper.Path.Arc(startPoint, midPoint, endPoint)
        fanShape.add(position)
        fanShape.closed = true
        return fanShape
    }

    drawDonut(position: paper.Point, rsmall: number, rbig: number, startAngle: number = 0.0, endAngle: number = 360) {

        let p1 = this.drawFanShape(position, rsmall, startAngle, endAngle)
        let p2 = this.drawFanShape(position, rbig, startAngle, endAngle)

        let donut = p2.subtract(p1)

        p1.remove()
        p2.remove()
        return donut
    }

    drawRotationIndicator(position, degree: number) {
        this.clearRotationIndicator()

        let circles = degree > 0?Math.floor(degree / 360):Math.ceil(degree/360);
        let residual = degree - circles * 360;

        let radius1 = 20.0;
        let radius2 = 25.0;

        let fillColor = degree > 0 ? new paper.Color("yellow") : new paper.Color("red")
        let radiusStep = 6.0

        for (let circleIdx = 0; circleIdx < Math.abs(circles); circleIdx++) {
            let circleShape = this.drawDonut(position, radius1, radius2)

            circleShape.fillColor = fillColor
            radius1 += radiusStep
            radius2 += radiusStep;

            this.rotationIndicator.push(circleShape)
        }

        let startAngle = Math.atan2(this.startPos.y - position.y, this.startPos.x - position.x)

        let residualFanShape = this.drawDonut(position, radius1, radius2, startAngle, startAngle + residual)
        residualFanShape.fillColor = fillColor
        this.rotationIndicator.push(residualFanShape)
    }

    dragging(pos) {
        super.dragging(pos);

        if (this.isDragging && this.targetShape != null) {
            let vec1 = this.lastPos.subtract(this.targetShape.pivotPosition)
            let vec2 = pos.subtract(this.targetShape.pivotPosition)

            let theta = vec1.getDirectedAngle(vec2)
            this.rotationDegree += theta

            if(this.pressingShift){
                console.log("Last rotation degree:" + this.lastRotationDegree)
                console.log("Original theta:" + theta)
                console.log("Rotation degree:" + this.rotationDegree)
                let targetRotationDegree = Math.floor( this.rotationDegree / this.gapDegree ) * this.gapDegree
                console.log("Target rotation degree:" + targetRotationDegree)

                theta = targetRotationDegree - this.lastRotationDegree
                console.log("Theta:" + theta)
            }

            this.targetShape.rotateAroundPivot(theta)
            this.lastRotationDegree += theta

            this.lastPos = new paper.Point(pos.x, pos.y)
            this.targetShape.store()
            this.targetShape.update()

            this.drawRotationIndicator(this.targetShape.pivotPosition, this.lastRotationDegree)
        }
    }

    endMove() {
        this.clearRotationIndicator()

        let command = new ShapeRotateCommand(this.targetShape, this.rotationDegree)
        undoManager.PushCommand(command)
    }
}

let shapeRotateHandler = new ShapeRotateHandler()
export {shapeRotateHandler}