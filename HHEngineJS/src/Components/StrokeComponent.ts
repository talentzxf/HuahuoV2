import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {FloatPropertyConfig} from "hhcommoncomponents/dist/src/Properties/PropertyConfig";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {MirrorComponent} from "./MirrorComponent";

let componentName = "StrokeComponent"
class StrokeComponent extends AbstractComponent{
    static createComponent(rawObj){
        return new StrokeComponent(rawObj)
    }

    constructor(rawObj?) {
        super(rawObj);

        this.rawObj.SetTypeName(componentName)
    }

    @PropertyValue(PropertyCategory.interpolateFloat, 1, {min: 0.0} as FloatPropertyConfig)
    strokeWidth

    override afterUpdate() {
        super.afterUpdate();

        this.baseShape.paperShape.strokeWidth = this.strokeWidth
    }
}

clzObjectFactory.RegisterClass(componentName, StrokeComponent.createComponent)

export {StrokeComponent}