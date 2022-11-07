import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyType} from "hhcommoncomponents";

const propertyPrefix = "inspector.property."

enum PropertyCategory{
    interpolateFloat,
    interpolateColor,
    interpolateVector2,
    shapeArray
}

abstract class InterpolateOperator{
    rawObj
    constructor(rawObj) {
        this.rawObj = rawObj
    }

    abstract registerField(fieldName: string, initValue)
    abstract getField(fieldName: string)
    abstract setField(fieldName: string, val)
}

class InterpolateFloatOperator extends InterpolateOperator{
    getField(fieldName) {
        return this.rawObj["GetFloatValue"]()
    }

    registerField(fieldName, initValue) {
        this.rawObj["RegisterFloatValue"](fieldName)
    }

    setField(fieldName: string, val) {
        this.rawObj["SetFloatValue"](fieldName, val)
    }
}

class InterpolateColorOperator extends InterpolateOperator{
    getField(fieldName) {
        return this.rawObj["GetColorValue"]()
    }

    registerField(fieldName, initValue) {
        this.rawObj["RegisterColorValue"](fieldName)
    }

    setField(fieldName: string, val) {
        this.rawObj["SetColorValue"](fieldName, val.r, val.g, val.b, val.a)
    }
}

function buildOperator(type, rawObj): InterpolateOperator{
    switch(type){ // TODO: Get rid of switch-case
        case PropertyCategory.interpolateFloat:
            return new InterpolateFloatOperator(rawObj)
        case PropertyCategory.interpolateColor:
            return new InterpolateColorOperator(rawObj)
    }

    return null
}

class PropertyDef{
    key: string
    type: PropertyCategory
    config: object
    initValue: object|number
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
            unregisterValueChagneFunc: valueChangeHandler.unregisterValueChangeHandler(fieldName),
            config: propertyMeta.config
        }

        return propertyDef
    }
}

class InterpolatePropertyBuilder extends PropertySheetBuilder{
    override build(component, propertyMeta, valueChangeHandler: ValueChangeHandler) {
        let propertyDef = super.build(component, propertyMeta, valueChangeHandler);

        propertyDef["type"] = PropertyType.FLOAT
        propertyDef["elementType"] = "range"

        return propertyDef
    }
}

export {PropertyCategory, PropertyDef, capitalizeFirstLetter, PropertySheetBuilder, InterpolatePropertyBuilder, buildOperator}

