import {BaseShapeJS} from "./BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"
import {StrokeComponent} from "../Components/StrokeComponent";

abstract class BaseSolidShape extends BaseShapeJS {
    constructor(rawObj?) {
        let needInitComponents = false
        if(!rawObj){
            needInitComponents = true
        }

        super(rawObj);

        if(needInitComponents){
            this.addComponent(new StrokeComponent())
        }
    }

    get color(): paper.Color{
        return this.paperItem.fillColor
    }

    set color(val: paper.Color) {
        if(val.equals(this.paperItem.fillColor))
            return

        this.paperItem.fillColor = val
        this.callHandlers("color", val)
    }

    get strokeColor(): paper.Color{
        return this.paperItem.strokeColor
    }

    set strokeColor(val: paper.Color) {
        if(val.equals(this.paperItem.strokeColor))
            return

        this.paperItem.strokeColor = val
        this.callHandlers("strokeColor", val)
    }

    private setFillColor(val: paper.Color) {
        if (this.paperItem.fillColor != val) {
            this.paperItem.fillColor = val
            this.store()
        }
    }

    private getFillColor(): paper.Color {
        return this.paperItem.fillColor
    }

    private setStrokeColor(val: paper.Color) {
        if (this.paperItem.strokeColor != val) {
            this.paperItem.strokeColor = val
            this.store()
        }
    }

    private getStrokeColor(): paper.Color {
        return this.paperItem.strokeColor
    }

    afterWASMReady() {
        super.afterWASMReady();

        this.propertySheet.addProperty({
            key: "inspector.FillColor",
            type: PropertyType.COLOR,
            getter: this.getFillColor.bind(this),
            setter: this.setFillColor.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("color"),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("color")
        })

        this.propertySheet.addProperty({
            key:"inspector.StrokeColor",
            type: PropertyType.COLOR,
            getter: this.getStrokeColor.bind(this),
            setter: this.setStrokeColor.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("strokeColor"),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("strokeColor")
        })
    }

    store() {
        super.store();

        // Store color
        let fillColor = this.paperItem.fillColor
        if (fillColor) // Some shapes doesn't have fille color
            this.rawObj.SetColor(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha)

        // Store stroke color
        let strokeColor = this.paperItem.strokeColor
        if(strokeColor)
            this.rawObj.SetStrokeColor(strokeColor.red, strokeColor.green, strokeColor.blue, strokeColor.alpha)
    }

    afterUpdate() {
        super.afterUpdate();

        let rawFillColor = this.rawObj.GetColor()
        this.color = new paper.Color(rawFillColor.r, rawFillColor.g, rawFillColor.b, rawFillColor.a)

        let strokeColor = this.rawObj.GetStrokeColor()
        this.strokeColor = new paper.Color(strokeColor.r, strokeColor.g, strokeColor.b, strokeColor.a)
    }
}

export {BaseSolidShape}