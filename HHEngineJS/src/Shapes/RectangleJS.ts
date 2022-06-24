import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {Vector2} from "hhcommoncomponents"
declare var Module: any;
declare function castObject(obj:any, clz:any): any;

let shapeName = "RectangleShape"
class RectangleJS extends BaseShapeJS{
    static createRectangle(rawObj){
        return new RectangleJS(rawObj)
    }

    afterWASMReady() {
        super.afterWASMReady()
        this.rawObj = castObject(this.rawObj, Module.RectangleShape)
    }

    getShapeName(): string {
        return shapeName
    }

    createShape(){
        let p1 = this.getPaperPoint(this.rawObj.GetStartPoint())
        let p2 = this.getPaperPoint(this.rawObj.GetEndPoint())

        let paperjs = this.getPaperJs()
        this.paperShape = new paperjs.Path.Rectangle(p1, p2)
        this.paperShape.applyMatrix = false;
        this.paperShape.strokeColor = new paperjs.Color("black");
        this.paperShape.fillColor = this.color
        this.paperShape.data.meta = this
    }

    setStartPoint(startPoint: Vector2){
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0)
    }

    setEndPoint(endPoint: Vector2){
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);

        if(this.paperShape == null){
            this.color = paper.Color.random()
        }else{
            this.paperShape.remove()
        }

        this.createShape()
        this.store()
    }
}

shapeFactory.RegisterClass(shapeName, RectangleJS.createRectangle)

export {RectangleJS}