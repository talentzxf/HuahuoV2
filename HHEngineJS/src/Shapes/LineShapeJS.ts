import {BaseShapeJS} from "./BaseShapeJS";
import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";

class LineShapeJS extends BaseShapeJS{
    startPoint: Vector2
    endPoint: Vector2

    line: paper.Path.Line

    getShapeName(){
        return "LineShape"
    }

    update() {
        if(this.line == null){
            this.line = new paper.Path.Line( this.startPoint, this.endPoint)
            this.line.strokeColor = new paper.Color("black")
        }else{
            this.line.removeSegments()
            this.line.add(this.startPoint, this.endPoint)
        }
    }

    setStartPoint(startPoint: Vector2){
        this.startPoint = startPoint
    }

    setEndPoint(endPoint: Vector2){
        this.endPoint = endPoint
    }
}

export {LineShapeJS}