import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import * as paper from "paper";
import {LineShapeJS} from "./LineShapeJS";
declare function castObject(obj:any, clz:any): any;
declare var Module: any;

let shapeName = "CircleShape"
class CircleShapeJS extends BaseShapeJS{
    circle: paper.Path.Circle

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

    update(){
        let circleCenter = this.getPaperPoint(this.rawObj.GetCenter());
        let radius = this.rawObj.GetRadius();
        if(this.circle){
            this.circle.remove()
        }

        this.circle = new paper.Path.Circle(circleCenter, radius);
        this.circle.strokeColor = new paper.Color("black")
    }
}

shapeFactory.RegisterClass(shapeName, CircleShapeJS.createCircle)


export {CircleShapeJS}
