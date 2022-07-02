import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";

let shapeName = "ElementShape"
class ElementShapeJS extends BaseShapeJS{
    static createElement(rawObj){
        return new ElementShapeJS(rawObj)
    }

    storeId: number = -1;
    size: paper.Point

    constructor(rawObj) {
        super(rawObj);

        this.size = new paper.Point(100, 100)
    }
    getShapeName(): string {
        return shapeName
    }

    // Render a box first.
    createShape() {
        let paper = this.getPaperJs()
        this.paperItem = new paper.Group()
        this.paperItem.applyMatrix = false
        this.paperItem.data.meta = this

        let p1 = new paper.Point(0,0)
        let p2 = this.size
        let boundingBox = new paper.Path.Rectangle(p1, p2)
        boundingBox.strokeColor = new paper.Color("black")
        this.paperItem.addChild(boundingBox)
    }
}

shapeFactory.RegisterClass(shapeName, ElementShapeJS.createElement)
export {ElementShapeJS}