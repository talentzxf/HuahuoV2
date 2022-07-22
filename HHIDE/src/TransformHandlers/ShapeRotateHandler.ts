import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {paper, BaseShapeJS} from "hhenginejs"
import {Vector2} from "hhcommoncomponents"

class ShapeRotateHandler extends ShapeTranslateMorphBase {
    protected rotationCenter: paper.Point
    protected targetShape: BaseShapeJS
    protected lastPos: paper.Point = null

    private rotationDegree: number = 0.0;

    private rotationIndicator: Array<paper.Path> = new Array<paper.Path>()

    constructor() {
        super();
    }

    beginMove(startPos) {
        super.beginMove(startPos);
        this.lastPos = new paper.Point(startPos.x, startPos.y)
        this.targetShape = this.curObjs.values().next().value // There's only one object in the set, get it.

        this.rotationCenter = this.targetShape.position

        this.rotationDegree = 0.0;
    }

    clearRotationIndicator() {
        for (let shape of this.rotationIndicator) {
            shape.remove()
        }

        this.rotationIndicator = new Array<paper.Path>()
    }

    drawFanShape(position: paper.Point, radius: number, startAngle: number, endAngle: number) {
        if(startAngle == 0 && endAngle == 360){
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

        let triangle = paper.Path.Triangle()
        fanShape.closed = true

        return fanShape
    }

    drawDonut(position: paper.Point, rsmall: number, rbig: number, startAngle: number = 0.0, endAngle: number = 360) {

        let p1 = this.drawFanShape(position, rsmall, startAngle, endAngle)
        let p2 = this.drawFanShape(position, rbig, startAngle, endAngle)

        let donut = p2.subtract(p1)

        p1.remove()
        p2.remove()

        donut.fillColor = new paper.Color("yellow")
        return donut
    }

    drawRotationIndicator(position, degree: number) {
        this.clearRotationIndicator()

        if (degree > 0) {
            let circles = Math.floor(degree / 360);
            let residual = degree - circles * 360;

            let radius1 = 20.0;
            let radius2 = 25.0;

            for (let circleIdx = 0; circleIdx < circles; circleIdx++) {
                let circleShape = this.drawDonut(position, radius1, radius2)
                circleShape.fillColor = new paper.Color("yellow")
                radius1 += 6.0;
                radius2 += 6.0;
                this.rotationIndicator.push(circleShape)
            }

            let startAngle = Math.atan2(this.startPos.y - position.y, this.startPos.x - position.x)

            let residualFanShape = this.drawDonut(position, radius1, radius2, startAngle, startAngle + residual)
            this.rotationIndicator.push(residualFanShape)
        }
    }

    dragging(pos) {
        super.dragging(pos);

        if (this.isDragging && this.targetShape != null) {
            let vec1 = this.lastPos.subtract(this.rotationCenter)
            let vec2 = pos.subtract(this.rotationCenter)

            let theta = vec1.getDirectedAngle(vec2)

            this.rotationDegree += theta

            this.targetShape.rotate(theta, this.rotationCenter)
            this.lastPos = new paper.Point(pos.x, pos.y)
            this.targetShape.store()
            this.targetShape.updateBoundingBox()

            this.drawRotationIndicator(this.targetShape.position, this.rotationDegree)
        }
    }

    endMove() {
        this.clearRotationIndicator()
    }
}

let shapeRotateHandler = new ShapeRotateHandler()
export {shapeRotateHandler}