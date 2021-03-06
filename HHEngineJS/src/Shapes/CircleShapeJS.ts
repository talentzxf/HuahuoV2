import {shapeFactory} from "./BaseShapeJS";
import * as paper from "paper";
import {BaseSolidShape} from "./BaseSolidShape";

let shapeName = "CircleShape"
class CircleShapeJS extends BaseSolidShape{

    static createCircle(rawObj){
        return new CircleShapeJS(rawObj);
    }

    randomColor: paper.Color

    getShapeName(){
        return shapeName
    }

    setCenter(center){
        this.rawObj.SetCenter(center.x, center.y, 0);
    }

    createShape(){
        let circleCenter = this.getPaperPoint(this.rawObj.GetCenter());
        let radius = this.rawObj.GetRadius()

        let paperjs = this.getPaperJs()
        this.paperShape = new paperjs.Path.Circle(circleCenter, radius);
        this.paperShape.applyMatrix = false;
        this.paperShape.strokeColor = new paper.Color("black")
        this.paperShape.fillColor = this.randomColor
        this.paperShape.data.meta = this
    }

    setRadius(radius){
        this.rawObj.SetRadius(radius)

        if(this.paperShape == null){
            this.randomColor = paper.Color.random()
        } else {
            this.paperShape.remove()
        }

        this.createShape()

        this.store()
    }
}

shapeFactory.RegisterClass(shapeName, CircleShapeJS.createCircle)


export {CircleShapeJS}
