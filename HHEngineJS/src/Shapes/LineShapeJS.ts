import {Vector2, PropertyType} from "hhcommoncomponents"
import * as paper from "paper";
import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";

let shapeName = "LineShape"

class LineShapeJS extends BaseShapeJS{
    static createLine(rawObj){
        return new LineShapeJS(rawObj);
    }

    randomStrokeColor: paper.Color

    getShapeName(){
        return shapeName
    }

    createShape()
    {
        super.createShape()

        let p1 = this.getPaperPoint(this.rawObj.GetStartPoint());
        let p2 = this.getPaperPoint(this.rawObj.GetEndPoint());

        let paperjs = this.getPaperJs()
        this.paperShape = new paperjs.Path.Line( p1, p2);
        this.paperShape.applyMatrix = false;
        this.paperShape.strokeColor = this.randomStrokeColor
        this.paperShape.data.meta = this
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

    getStrokeWidth(){
        return this.paperShape.strokeWidth
    }

    setStrokeWidth(val){
        this.paperShape.strokeWidth = val
    }

    afterWASMReady() {
        super.afterWASMReady();
        this.propertySheet.addProperty({
            key: "StrokeWidth",
            type: PropertyType.FLOAT,
            getter: this.getStrokeWidth.bind(this),
            setter: this.setStrokeWidth.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("strokeWidth").bind(this),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("strokeWidth").bind(this)
        })
    }
}

shapeFactory.RegisterClass(shapeName, LineShapeJS.createLine)

export {LineShapeJS}