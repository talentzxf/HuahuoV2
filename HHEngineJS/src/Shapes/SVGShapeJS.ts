import {BaseSolidShape} from "./BaseSolidShape";

let shapeName = "BaseShape"
class SVGShapeJS extends BaseSolidShape{
    static createShape(rawObj){
        return new SVGShapeJS()
    }

    shapeURL: string
    getShapeName(): string {
        return shapeName;
    }

    setShapeURL(shapeURL){
        this.shapeURL = shapeURL
    }

    createShape(){
        super.createShape()

        let _this = this
        let paperJs = this.getPaperJs()
        paperJs["project"]["importSVG"](this.shapeURL, function(item){
            _this.paperShape = item
            _this.paperShape.applyMatrix = false
            // _this.paperShape.strokeColor = new paper.Color("black")
            // _this.paperShape.fillColor = new paper.Color("green")
            _this.paperShape.data.meta = this

            _this.afterCreateShape()
        })
    }

    update(){
        if(this.paperShape){
            super.update()
        }
    }
}

export {SVGShapeJS}