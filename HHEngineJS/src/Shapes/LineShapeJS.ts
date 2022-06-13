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

    awakeFromLoad(){
        this.update();
    }

    getShapeName(){
        return shapeName
    }

    duringUpdate() {
        super.duringUpdate()

        let startPaperPoint = this.getPaperPoint(this.rawObj.GetStartPoint());
        let endPaperPoint = this.getPaperPoint(this.rawObj.GetEndPoint());

        this.paperShape = new paper.Path.Line( startPaperPoint, endPaperPoint);
        this.paperShape.data.meta = this
        this.paperShape.strokeColor = paper.Color.random()
    }

    setStartPoint(startPoint: Vector2){
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0);
    }

    setEndPoint(endPoint: Vector2){
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);
    }
}

shapeFactory.RegisterClass(shapeName, LineShapeJS.createLine)

export {LineShapeJS}