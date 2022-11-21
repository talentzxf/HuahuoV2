import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyType} from "hhcommoncomponents";

const eps:number = 0.001;

enum PropertyCategory{
    interpolateFloat,
    interpolateColor,
    interpolateVector2,  // vector2 is just vector3 with z = 0
    interpolateVector3,
    shapeArray,
    colorStopArray, // Every color stop is a float->Color mapping entry.
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

class InterpolateVector2Operator extends InterpolateOperator{
    getField(fieldName: string) {
        return this.rawObj["GetVector3Value"](fieldName)
    }

    isEqual(v1, v2) {
        if(Math.abs(v1.x - v2.x) <= eps && Math.abs(v1.y - v2.y) <= eps)
            return true
        return false
    }

    registerField(fieldName: string, initValue) {
        if(!initValue){
            initValue = {
                x: 0.0,
                y: 0.0
            }
        }
        this.rawObj["RegisterVector3Value"](fieldName, initValue.x, initValue.y, 0.0)
    }

    setField(fieldName: string, val) {
        this.rawObj["SetVector3Value"](fieldName, val.x, val.y, 0.0)
    }
}

class InterpolateVector3Operator extends InterpolateOperator{
    getField(fieldName: string) {
        return this.rawObj["GetVector3Value"](fieldName)
    }

    isEqual(v1, v2) {
        if(Math.abs(v1.x - v2.x) <= eps && Math.abs(v1.y - v2.y) <= eps && Math.abs(v1.z - v2.z) <= eps)
            return true
        return false
    }

    registerField(fieldName: string, initValue) {
        if(!initValue){
            initValue = {
                x: 0.0,
                y: 0.0,
                z: 0.0
            }
        }
        this.rawObj["RegisterVector3Value"](fieldName, initValue.x, initValue.y, initValue.z)
    }

    setField(fieldName: string, val) {
        this.rawObj["SetVector3Value"](fieldName, val.x, val.y, val.z)
    }
}

function buildOperator(type, rawObj): InterpolateOperator{
    switch(type){ // TODO: Get rid of switch-case
        case PropertyCategory.interpolateFloat:
            return new InterpolateFloatOperator(rawObj)
        case PropertyCategory.interpolateColor:
            return new InterpolateColorOperator(rawObj)
        case PropertyCategory.interpolateVector2:
            return new InterpolateVector2Operator(rawObj)
        case PropertyCategory.interpolateVector3:
            return new InterpolateVector3Operator(rawObj)
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

export {PropertyCategory, PropertyDef, capitalizeFirstLetter, buildOperator}

