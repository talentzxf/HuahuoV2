import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";
import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
declare var Module: any;
declare function castObject(obj:any, clz:any): any;

let shapeName = "LineShape"

class LineShapeJS extends BaseShapeJS{
    line: paper.Path.Line

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

    update() {
        let startPaperPoint = this.getPaperPoint(this.rawObj.GetStartPoint());
        let endPaperPoint = this.getPaperPoint(this.rawObj.GetEndPoint());

        if(this.line == null){
            this.line = new paper.Path.Line( startPaperPoint, endPaperPoint);
            this.line.strokeColor = new paper.Color("black")
        }else{
            this.line.removeSegments()
            this.line.add(startPaperPoint, endPaperPoint)
        }
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