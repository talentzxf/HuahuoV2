import {BaseSolidShape} from "./BaseSolidShape";
import {PropertyType} from "hhcommoncomponents";
import {shapeFactory} from "./BaseShapeJS";

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
        let paperjs = this.getPaperJs()
        this.paperShape = new paperjs.Path({
            segments: [],
            strokeColor: 'black',
            fullySelected: true
        })
        this.paperShape.applyMatrix = false

        this.paperShape.data.meta = this
    }

    addPoint(p:paper.Point){
        if(this.paperShape == null){
            this.createShape()
        }

        this.paperShape.add(p)
    }

    endDrawingCurve(){
        this.paperShape.simplify(10)
        super.afterCreateShape()

        this.store()
    }

    encloseCurve(){
        this.paperShape.closed = true
    }

    growth: number = 1.0

    lastGrowthNumber: number = -1.0
    getGrowth(){
        return this.growth
    }

    setGrowth(val:number){
        this.growth = val
        this.callHandlers("growth", val)
        this.update()
    }

    afterUpdate() {
        if(this.growth >= 1.0)
            super.afterUpdate();

        if(this.growth < 1.0 && this.lastGrowthNumber != this.growth){
            super.afterUpdate()

            let path2 = this.paperShape.splitAt(this.paperShape.length * this.growth)
            path2.visible = false
            path2.selected = false

            this.lastGrowthNumber = this.growth

        }
    }

    afterWASMReady() {
        super.afterWASMReady();

        this.propertySheet.addProperty({
            key: "inspector.enclose",
            type: PropertyType.BUTTON,
            action: this.encloseCurve.bind(this)
        })

        this.propertySheet.addProperty({
            key:"inspector.growth",
            type: PropertyType.FLOAT,
            getter: this.getGrowth.bind(this),
            setter: this.setGrowth.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("growth").bind(this),
            unregisterValueChagneFunc: this.unregisterValueChangeHandler("growth").bind(this)
        })
    }
}

shapeFactory.RegisterClass(shapeName, CurveShapeJS.createCurveShape)
export {CurveShapeJS}