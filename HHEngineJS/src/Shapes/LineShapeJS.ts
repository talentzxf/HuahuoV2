import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";
import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
declare var Module: any;
declare function castObject(obj:any, clz:any): any;

let shapeName = "LineShape"

class LineShapeJS extends BaseShapeJS{
    static createLine(rawObj){
        return new LineShapeJS(rawObj);
    }

    afterWASMReady(){
        this.rawObj = castObject(this.rawObj, Module.LineShape);
    }

    getShapeName(){
        return shapeName
    }

    duringUpdate() {
        super.duringUpdate()

        let startPaperPoint = this.getPaperPoint(this.rawObj.GetStartPoint());
        let endPaperPoint = this.getPaperPoint(this.rawObj.GetEndPoint());

        if(this.paperShape == null){
            this.paperShape = new paper.Path.Line( startPaperPoint, endPaperPoint);
            this.paperShape.data.meta = this
            this.paperShape.strokeColor = paper.Color.random()
            this.color = this.paperShape.strokeColor
        } else {
            this.paperShape = new paper.Path.Line( startPaperPoint, endPaperPoint);
            this.paperShape.data.meta = this
            this.paperShape.strokeColor = this.color
        }
    }

    setStartPoint(startPoint: Vector2){
        this.position = startPoint
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0);
    }

    setEndPoint(endPoint: Vector2){
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);

        let p1 = this.rawObj.GetStartPoint()
        let p2 = this.rawObj.GetEndPoint()
        this.position = new Vector2((p1.x + p2.x)/2.0,(p1.y + p2.y)/2.0)
    }
}

shapeFactory.RegisterClass(shapeName, LineShapeJS.createLine)

export {LineShapeJS}