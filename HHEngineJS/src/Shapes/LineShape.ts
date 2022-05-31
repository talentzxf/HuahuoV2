import {BaseShape} from "./BaseShape";
import {Vector2} from "hhcommoncomponents"
import {DrawUtils} from "../DrawUtils/DrawUtils";
import * as paper from "paper";

class LineShape extends BaseShape{
    startPoint: Vector2
    endPoint: Vector2

    line: paper.Path.Line

    update() {
        let globalStartPoint = this.getGlobalPositionFromVec(this.startPoint)
        let globalEndPoint = this.getGlobalPositionFromVec(this.endPoint)

        if(this.line == null){
            this.line = new paper.Path.Line( globalStartPoint, globalEndPoint)
            this.line.strokeColor = new paper.Color("black")
        }else{
            this.line.removeSegments()
            this.line.add(globalStartPoint, globalEndPoint)
        }
    }

    setStartPoint(startPoint: Vector2){
        this.startPoint = startPoint
    }

    setEndPoint(endPoint: Vector2){
        this.endPoint = endPoint
    }
}

export {LineShape}