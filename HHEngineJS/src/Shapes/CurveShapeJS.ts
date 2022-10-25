import {BaseSolidShape} from "./BaseSolidShape";
import {PropertyType} from "hhcommoncomponents";
import {CurveGrowthComponent} from "../Components/CurveGrowthComponent";
import {clzObjectFactory} from "../CppClassObjectFactory";

let shapeName = "CurveShape"
class CurveShapeJS extends BaseSolidShape{
    static createCurveShape(rawObj){
        return new CurveShapeJS(rawObj)
    }

    curveGrowthComponent: CurveGrowthComponent = new CurveGrowthComponent()

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

    get growth(){
        return this.curveGrowthComponent.growth
    }

    set growth(val:number){
        this.curveGrowthComponent["setGrowth"](val)
    }

    getGrowth(){
        return this.growth
    }

    setGrowth(val:number){
        this.growth = val
        this.callHandlers("growth", val)
        this.update()
    }

    // This might be overridden by CurveShape. Because we want to implement the growth factor.
    getSegments(){
        return this.curveGrowthComponent.getSegments()
    }

    afterUpdate() {
        super.afterUpdate()
    }

    afterWASMReady() {
        super.afterWASMReady();

        this.propertySheet.addProperty({
            key: "inspector.enclose",
            type: PropertyType.BUTTON,
            action: this.encloseCurve.bind(this)
        })

        this.propertySheet.addProperty({
            key:"inspector.curve.growth",
            type: PropertyType.FLOAT,
            elementType: "range",
            min: 0.0,
            max: 1.0,
            step: 0.01,
            getter: this.getGrowth.bind(this),
            setter: this.setGrowth.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("growth"),
            unregisterValueChagneFunc: this.unregisterValueChangeHandler("growth")
        })
    }
}

clzObjectFactory.RegisterClass(shapeName, CurveShapeJS.createCurveShape)
export {CurveShapeJS}