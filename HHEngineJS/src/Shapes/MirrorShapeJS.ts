import {BaseShapeJS} from "./BaseShapeJS";
import * as paper from "paper";
import {Vector2} from "hhcommoncomponents"

let shapeName = "MirrorShape"
class MirrorShapeJS extends BaseShapeJS{
    static createMirror(rawObj){
        return new MirrorShapeJS(rawObj)
    }

    randomStrokeColor: paper.Color

    getShapeName(): string {
        return shapeName
    }

    createShape(){
        super.createShape()

        let p1 = this.getPaperPoint(this.rawObj.GetStartPoint())
        let p2 = this.getPaperPoint(this.rawObj.GetEndPoint())

        let paperjs = this.getPaperJs()

        this.paperShape = new paperjs.Path.Line( p1, p2)
        this.paperShape.applyMatrix = false;
        this.paperShape.strokeColor = this.randomStrokeColor;
        this.paperShape.data.meta = this

        super.afterCreateShape()
    }


    setStartPoint(startPoint: Vector2){
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0);
    }

    setEndPoint(endPoint: Vector2){
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);

        if(this.paperShape == null){
            this.randomStrokeColor = paper.Color.random()
        } else {
            this.paperShape.remove()
        }

        this.createShape()
        this.store()
    }

    override hitTypeSelectable(hitType): boolean {
        if(hitType == "stroke")
            return false
        return true
    }
}

export {MirrorShapeJS}