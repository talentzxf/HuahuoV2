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

    private setFillColor(val: paper.Color) {
        if (this.paperItem.fillColor != val) {
            this.paperItem.fillColor = val
            this.store()
        }
    }

    private getFillColor(): paper.Color {
        return this.paperItem.fillColor
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
    }

    store() {
        super.store();

        // Store color
        let fillColor = this.paperItem.fillColor
        if (fillColor) // Some shapes doesn't have fille color
            this.rawObj.SetColor(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha)
    }

    afterUpdate() {
        super.afterUpdate();

        let rawFillColor = this.rawObj.GetColor()
        this.color = new paper.Color(rawFillColor.r, rawFillColor.g, rawFillColor.b, rawFillColor.a)
    }
}

export {BaseSolidShape}