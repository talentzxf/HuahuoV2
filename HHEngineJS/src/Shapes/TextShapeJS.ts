import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {PropertyType} from "hhcommoncomponents";

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

        let textItem = this.paperItem as paper.PointText
        if(textItem && textItem.content != val){
            textItem.content = val
        }
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

    setTextWithPosition(inText:string, position: Vector2){
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

    getText(){
        return this.text
    }

    setText(inText:string){
        this.text = inText

        this.updateBoundingBox()
    }

    afterWASMReady() {
        super.afterWASMReady();

        this.propertySheet.addProperty({
            key: "Text",
            type: PropertyType.STRING,
            getter: this.getText.bind(this),
            setter: this.setText.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("text").bind(this),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("text").bind(this)
        })
    }
}

shapeFactory.RegisterClass(shapeName, TextShapeJS.createTextShape)

export {TextShapeJS}