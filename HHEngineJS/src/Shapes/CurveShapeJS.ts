import {BaseSolidShape} from "./BaseSolidShape";
import {PropertyType} from "hhcommoncomponents";
import {CurveGrowthComponent} from "../Components/CurveGrowthComponent";
import {clzObjectFactory} from "../CppClassObjectFactory";
import Curve = paper.Curve;

let shapeName = "CurveShape"
class CurveShapeJS extends BaseSolidShape{
    static createCurveShape(rawObj){
        return new CurveShapeJS(rawObj)
    }

    constructor(rawObj?) {
        let needInitComponents = false
        if(!rawObj){
            needInitComponents = true
        }

        super(rawObj);

        if(needInitComponents){
            this.curveGrowthComponent = new CurveGrowthComponent()
            this.addComponent(this.curveGrowthComponent)
        }
    }

    override awakeFromLoad() {
        super.awakeFromLoad();

        if(this.curveGrowthComponent){
            this.curveGrowthComponent = this.getComponentByTypeName("CurveGrowthComponent") as CurveGrowthComponent
        }
    }

    curveGrowthComponent: CurveGrowthComponent

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
        if(this.curveGrowthComponent)
            return this.curveGrowthComponent.growth
        return 1.0
    }

    set growth(val:number){
        if(this.curveGrowthComponent)
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
        if(this.curveGrowthComponent)
            return this.curveGrowthComponent.getSegments()
        return this.paperShape.segments
    }

    override setSegmentProperty(idx, property, value) {
        super.setSegmentProperty(idx, property, value);

        if(this.curveGrowthComponent)
            this.curveGrowthComponent.setSegmentProperty(idx, property, value)
    }

    override insertSegment(localPos: paper.Point) {
        super.insertSegment(localPos);

        if(this.curveGrowthComponent)
            this.curveGrowthComponent.insertSegment(localPos)
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