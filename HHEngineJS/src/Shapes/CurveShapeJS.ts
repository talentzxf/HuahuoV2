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

    _curveGrowthComponent: CurveGrowthComponent = null

    get curveGrowthComponent(){
        if(this._curveGrowthComponent == null){
            let component = this.getComponentByTypeName("CurveGrowthComponent") as CurveGrowthComponent
            if(component != null){
                this._curveGrowthComponent = component
            }
        }
        return this._curveGrowthComponent
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

        let fillComponent = this.getComponentByTypeName("FillColorComponent")
        if(fillComponent){
            let color = fillComponent["fillColor"]
            color.alpha = 0.0
            fillComponent["fillColor"] = color
        }

        this.store()
    }

    encloseCurve(){
        this.paperShape.closed = true
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

    afterWASMReady() {
        super.afterWASMReady();

        this.propertySheet.addProperty({
            key: "inspector.enclose",
            type: PropertyType.BUTTON,
            config:{
                action: this.encloseCurve.bind(this)
            }
        })
    }
}

clzObjectFactory.RegisterClass(shapeName, CurveShapeJS.createCurveShape)
export {CurveShapeJS}