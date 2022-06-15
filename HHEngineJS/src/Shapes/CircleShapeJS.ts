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

    getShapeName(){
        return shapeName
    }

    setCenter(center){
        this.position = center
        this.rawObj.SetCenter(center.x, center.y, 0);
    }

    createShape(){
        let circleCenter = this.getPaperPoint(this.rawObj.GetCenter());
        let radius = this.rawObj.GetRadius()
        this.paperShape = new paper.Path.Circle(circleCenter, radius);
        this.paperShape.strokeColor = new paper.Color("black")
        this.paperShape.fillColor = this.color
        this.paperShape.data.meta = this
    }

    setRadius(radius){
        this.rawObj.SetRadius(radius)

        if(this.paperShape == null){
            this.color = paper.Color.random()
        } else {
            this.paperShape.remove()
        }

        this.createShape()
        this.position = this.paperShape.position
    }

    duringUpdate(){
        super.duringUpdate()
    }
}

shapeFactory.RegisterClass(shapeName, CircleShapeJS.createCircle)


export {CircleShapeJS}
