import {BaseShapeJS} from "./BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

abstract class BaseSolidShape extends BaseShapeJS{
    private getFillColor(): paper.Color {
        return this.color
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
}

export {BaseSolidShape}