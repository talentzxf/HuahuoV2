import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyType} from "hhcommoncomponents";

const propertyPrefix = "inspector.property."

enum PropertyCategory{
    interpolate,
    shapeArray,
}

class PropertyDef{
    key: string
    type: PropertyCategory
    config: object
    initValue: object|number

    // minValue?: number = 0.0
    // maxValue?: number = 1.0
    // step?: number = 0.01
}

function capitalizeFirstLetter(str){
    if(str.length == 0)
        return ""

    return str.charAt(0).toUpperCase() + str.slice(1);
}

class PropertySheetBuilder{
    build(component, propertyMeta, valueChangeHandler: ValueChangeHandler){
        let fieldName = propertyMeta["key"]
        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName)

        let propertyDef = {
            key: propertyPrefix + propertyMeta["key"],
            getter: component[getterName].bind(component),
            setter: component[setterName].bind(component),
            registerValueChangeFunc: valueChangeHandler.registerValueChangeHandler(fieldName),
            unregisterValueChagneFunc: valueChangeHandler.unregisterValueChangeHandler(fieldName)
        }

        return propertyDef
    }
}

class InterpolatePropertyBuilder extends PropertySheetBuilder{
    override build(component, propertyMeta, valueChangeHandler: ValueChangeHandler) {
        let propertyDef = super.build(component, propertyMeta, valueChangeHandler);

        propertyDef["type"] = PropertyType.FLOAT
        propertyDef["elementType"] = "range"
        propertyDef["min"] = propertyMeta.config.minValue
        propertyDef["max"] = propertyMeta.config.maxValue
        propertyDef["step"] = propertyMeta.config.step

        return propertyDef
    }
}

export {PropertyCategory, PropertyDef, capitalizeFirstLetter, PropertySheetBuilder, InterpolatePropertyBuilder}

