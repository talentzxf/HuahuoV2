import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {StrokeComponent} from "./StrokeComponent";

let componentName = "FillColorComponent"

class FillColorComponent extends AbstractComponent{
    static createComponent(rawObj){
        return new FillColorComponent(rawObj)
    }

    constructor(rawObj?) {
        super(rawObj);
        this.rawObj.SetTypeName(componentName)
    }

    @PropertyValue(PropertyCategory.interpolateColor, {r:0, g:0, b:0, a:1})
    fillColor

    override afterUpdate() {
        super.afterUpdate();

        this.baseShape.paperShape.fillColor = this.fillColor
    }
}

clzObjectFactory.RegisterClass(componentName, FillColorComponent.createComponent)

export {FillColorComponent}