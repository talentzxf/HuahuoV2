import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import * as paper from "paper";
declare function castObject(obj:any, clz:any): any;
declare var Module: any;

let shapeName = "CircleShape"
class CircleShapeJS extends BaseShapeJS{

    static createCircle(rawObj){
        return new CircleShapeJS(rawObj);
    }

    afterWASMReady(){
        this.rawObj = castObject(this.rawObj, Module.CircleShape);
    }

    awakeFromLoad(){
        this.update();
    }

    getShapeName(){
        return shapeName
    }

    setCenter(center){
        this.rawObj.SetCenter(center.x, center.y, 0);
    }

    setRadius(radius){
        this.rawObj.SetRadius(radius)
    }

    duringUpdate(){
        super.duringUpdate()
        let circleCenter = this.getPaperPoint(this.rawObj.GetCenter());
        let radius = this.rawObj.GetRadius();

        this.paperShape = new paper.Path.Circle(circleCenter, radius);
        this.paperShape.data.meta = this
        this.paperShape.strokeColor = new paper.Color("black")
        this.paperShape.fillColor = paper.Color.random()
    }
}

shapeFactory.RegisterClass(shapeName, CircleShapeJS.createCircle)


export {CircleShapeJS}
