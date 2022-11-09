import * as paper from "paper";
import {BaseSolidShape} from "./BaseSolidShape";
import {clzObjectFactory} from "../CppClassObjectFactory";

let shapeName = "CircleShape"
class CircleShapeJS extends BaseSolidShape{

    static createCircle(rawObj){
        return new CircleShapeJS(rawObj);
    }

    getShapeName(){
        return shapeName
    }

    setCenter(center){
        this.rawObj.SetCenter(center.x, center.y, 0);
    }

    createShape(){
        super.createShape()

        let circleCenter = this.getPaperPoint(this.rawObj.GetCenter());
        let radius = this.rawObj.GetRadius()

        let paperjs = this.getPaperJs()
        this.paperShape = new paperjs.Path.Circle(circleCenter, radius);
        this.paperShape.applyMatrix = false;
        this.paperShape.data.meta = this

        super.afterCreateShape()
    }

    setRadius(radius){
        this.rawObj.SetRadius(radius)

        if(this.paperShape != null){
            this.paperShape.remove()
        }

        this.createShape()

        this.store()
    }
}

clzObjectFactory.RegisterClass(shapeName, CircleShapeJS.createCircle)
export {CircleShapeJS}
