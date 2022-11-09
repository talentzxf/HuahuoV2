import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyType} from "hhcommoncomponents";

const propertyPrefix = "inspector.property."
const eps:number = 0.001;

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
    abstract isEqual(v1, v2)
}

class InterpolateFloatOperator extends InterpolateOperator{
    getField(fieldName) {
        return this.rawObj["GetFloatValue"](fieldName)
    }

    registerField(fieldName, initValue) {
        this.rawObj["RegisterFloatValue"](fieldName, initValue)
    }

    setField(fieldName: string, val) {
        this.rawObj["SetFloatValue"](fieldName, val)
    }

    isEqual(val1, val2){
        return Math.abs(val1 - val2) < eps
    }
}

class InterpolateColorOperator extends InterpolateOperator{
    getField(fieldName) {
        let cppColor = this.rawObj["GetColorValue"](fieldName)
        return new paper.Color(cppColor.r, cppColor.g, cppColor.b, cppColor.a)
    }

    registerField(fieldName, initValue) {
        if(initValue["random"] == null || initValue["random"] == "false"){
            this.rawObj["RegisterColorValue"](fieldName, initValue.r, initValue.g, initValue.b, initValue.a)
        }else{
            let r = Math.random()
            let g = Math.random()
            let b = Math.random()
            this.rawObj["RegisterColorValue"](fieldName, r, g, b, 1.0)
        }

    }

    setField(fieldName: string, val) {
        this.rawObj["SetColorValue"](fieldName, val.red, val.green, val.blue, val.alpha)
    }

    isEqual(v1, v2) {
        let differenceSquare:number = (v1.red - v2.red)**2 + (v1.green - v2.green)**2 +
            (v1.blue - v2.blue)**2 + (v1.alpha - v2.alpha)**2
        if(Math.abs(differenceSquare) < eps**2)
            return true
        return false
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

        if(propertyMeta.type == PropertyCategory.interpolateFloat) {
            propertyDef["type"] = PropertyType.FLOAT
            propertyDef["elementType"] = "range"
        }
        else if(propertyMeta.type == PropertyCategory.interpolateColor)
            propertyDef["type"] = PropertyType.COLOR

        return propertyDef
    }
}

export {PropertyCategory, PropertyDef, capitalizeFirstLetter, PropertySheetBuilder, InterpolatePropertyBuilder, buildOperator}

