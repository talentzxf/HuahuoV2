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

    createShape()
    {
        let p1 = this.getPaperPoint(this.rawObj.GetStartPoint());
        let p2 = this.getPaperPoint(this.rawObj.GetEndPoint());
        this.paperShape = new this.paperjs.Path.Line( p1, p2);
        this.paperShape.strokeColor = this.color
        this.paperShape.data.meta = this
    }

    duringUpdate() {
        super.duringUpdate()
    }

    setStartPoint(startPoint: Vector2){
        this.position = startPoint
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0);
    }

    setEndPoint(endPoint: Vector2){
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);

        if(this.paperShape == null){
            this.color = this.paperjs.Color.random()
        } else {
            this.paperShape.remove()
        }

        this.createShape()
        this.position = this.paperShape.position
    }
}

shapeFactory.RegisterClass(shapeName, LineShapeJS.createLine)

export {LineShapeJS}