import {BaseSolidShape} from "./BaseSolidShape";

let shapeName = "CurveShape"
class CurveShapeJS extends BaseSolidShape{
    static createCurveShape(rawObj){
        return new CurveShapeJS(rawObj)
    }

    getShapeName(): string {
        return shapeName
    }

    createShape() {
        super.createShape();

        
    }
}