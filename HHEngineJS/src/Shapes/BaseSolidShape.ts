import {BaseShapeJS} from "./BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

abstract class BaseSolidShape extends BaseShapeJS {
    get color(): paper.Color{
        return this.paperItem.fillColor
    }

    set color(val: paper.Color) {
        this.paperItem.fillColor = val
        this.callHandlers("color", val)
    }

    get strokeColor(): paper.Color{
        return this.paperItem.strokeColor
    }

    set strokeColor(val: paper.Color) {
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
            key: "FillColor",
            type: PropertyType.COLOR,
            getter: this.getFillColor.bind(this),
            setter: this.setFillColor.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("color").bind(this),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("color").bind(this)
        })

        this.propertySheet.addProperty({
            key:"StrokeColor",
            type: PropertyType.COLOR,
            getter: this.getStrokeColor.bind(this),
            setter: this.setStrokeColor.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("strokeColor").bind(this),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("strokeColor").bind(this)
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