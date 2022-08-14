import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";

let shapeName = "TextShape"
class TextShapeJS extends BaseShapeJS{
    static createTextShape(rawObj){
        return new TextShapeJS(rawObj)
    }

    text:string = ""
    textPosition:paper.Point = null

    getShapeName(): string {
        return shapeName
    }

    createShape() {
        super.createShape();

        let paperjs = this.getPaperJs()
        this.paperItem = new paperjs.PointText(this.textPosition)

        let textItem = this.paperItem as paper.PointText
        textItem.justification = "center";
        textItem.fillColor = new paper.Color("black")
        textItem.content = this.text
    }

    setText(inText:string, position: Vector2){
        this.text = inText
        this.textPosition = position

        this.createShape()
        // this.store()
    }
}

shapeFactory.RegisterClass(shapeName, TextShapeJS.createTextShape)

export {TextShapeJS}