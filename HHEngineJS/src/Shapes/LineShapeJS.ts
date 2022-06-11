import {BaseShapeJS} from "./BaseShapeJS";
import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";

class LineShapeJS extends BaseShapeJS{
    line: paper.Path.Line

    getShapeName(){
        return "LineShape"
    }

    update() {
        if(this.line == null){
            this.line = new paper.Path.Line( this.cppShapeObj.GetStartPoint(), this.cppShapeObj.GetEndpoint())
            this.line.strokeColor = new paper.Color("black")
        }else{
            this.line.removeSegments()
            this.line.add(this.cppShapeObj.GetStartPoint(), this.cppShapeObj.GetEndPoint())
        }
    }

    setStartPoint(startPoint: Vector2){
        this.cppShapeObj.SetStartPoint(startPoint.x, startPoint.y, 0);
    }

    setEndPoint(endPoint: Vector2){
        this.cppShapeObj.SetStartPoint(endPoint.x, endPoint.y, 0);
    }
}

export {LineShapeJS}