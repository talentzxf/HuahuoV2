import {BaseSolidShape} from "./BaseSolidShape";

let shapeName = "BaseShape"

class SVGShapeJS extends BaseSolidShape {
    static createShape(rawObj) {
        return new SVGShapeJS()
    }

    shapeURL: string

    getShapeName(): string {
        return shapeName;
    }

    setShapeURL(shapeURL) {
        this.shapeURL = shapeURL
    }

    createShape() {
        super.createShape()

        let _this = this
        let paperJs = this.getPaperJs()
        paperJs["project"]["importSVG"](this.shapeURL, function (item) {
            _this.paperShape = item
            _this.paperShape.applyMatrix = false
            // _this.paperShape.strokeColor = new paper.Color("black")
            // _this.paperShape.fillColor = new paper.Color("green")
            _this.paperShape.data.meta = this

            // Recurisively set the meta.
            let shapeStack: Array<paper.Item> = new Array()
            shapeStack.push(_this.paperShape)
            while (shapeStack.length > 0) {
                let currentShape = shapeStack.pop()
                currentShape.data.meta = _this

                if(currentShape.children){
                    for (let child of currentShape.children) {
                        shapeStack.push(child)
                    }
                }
            }

            _this.afterCreateShape()
        })
    }

    update() {
        if (this.paperShape) {
            super.update()
        }
    }
}

export {SVGShapeJS}