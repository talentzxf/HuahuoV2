import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";

let shapeName = "TextShape"
class TextShapeJS extends BaseShapeJS{
    static createTextShape(rawObj){
        return new TextShapeJS(rawObj)
    }

    get text():string{
        return this.rawObj.GetText()
    }

    set text(val:string){
        this.rawObj.SetText(val)
    }

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
        textItem.fontSize = "20px"

        textItem.data.meta = this
    }

    setText(inText:string, position: Vector2){
        this.text = inText
        this.textPosition = position

        if(!this.paperItem){
            this.createShape()
        }else{
            let textItem = this.paperItem as paper.PointText
            textItem.content = inText;
        }

        this.store()
    }
}

shapeFactory.RegisterClass(shapeName, TextShapeJS.createTextShape)

export {TextShapeJS}