import {BaseShapeJS} from "./BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

abstract class BaseSolidShape extends BaseShapeJS{
    private getFillColor(): paper.Color {
        return this.paperItem.fillColor
    }

    set color(val: paper.Color) {
        this.paperItem.fillColor = val
        this.callHandlers("color", val)
    }

    private setFillColor(val: paper.Color) {
        if (this.paperItem.fillColor != val) {
            this.paperItem.fillColor = val
            this.store()
        }
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
    }

    store() {
        super.store();

        if (this.isUpdateFillColor()) {
            // Store color
            let fillColor = this.paperItem.fillColor
            if (fillColor) // Some shapes doesn't have fille color
                this.rawObj.SetColor(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha)
        }
    }

    afterUpdate() {
        super.afterUpdate();

        if (this.isUpdateFillColor()) {
            let rawFillColor = this.rawObj.GetColor()
            this.color = new paper.Color(rawFillColor.r, rawFillColor.g, rawFillColor.b, rawFillColor.a)
        }
    }
}

export {BaseSolidShape}