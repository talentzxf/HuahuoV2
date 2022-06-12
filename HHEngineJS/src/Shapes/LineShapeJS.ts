import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";
import {BaseShapeJS} from "./BaseShapeJS";
declare var Module: any;
declare function castObject(obj:any, clz:any): any;

class LineShapeJS extends BaseShapeJS{
    line: paper.Path.Line

    constructor(){
        super();
    }

    afterWASMReady(){
        this.rawObj = castObject(this.rawObj, Module.LineShape);
        this.rawObj.AwakeFromLoadInJS = this.awakeFromLoad.bind(this);
    }

    awakeFromLoad(){
        this.update();
    }

    getShapeName(){
        return "LineShape"
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

    getPaperPoint(engineV3Point){
        return new paper.Point(engineV3Point.x, engineV3Point.y)
    }
}

export {LineShapeJS}