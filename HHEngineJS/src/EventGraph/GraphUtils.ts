import {PropertyType} from "hhcommoncomponents";
import {PropertyCategory} from "../Components/PropertySheetBuilder";

// TODO: Having two functions seems stupid
function getLiteGraphTypeFromPropertyType(propertyType: PropertyType) {
    let returnType = ""
    switch (propertyType) {
        case PropertyType.NUMBER:
            returnType = "number"
            break;
        case PropertyType.STRING:
            returnType = "string"
            break;
        case PropertyType.BOOLEAN:
            returnType = "boolean"
            break;
        case PropertyType.VECTOR2:
            returnType = "vec2"
            break;
        case PropertyType.SHAPE:
            returnType = "shape"
            break;
    }

    return returnType
}

function getLiteGraphTypeFromPropertyCategory(propertyCategory: PropertyCategory): string {
    let returnType = ""
    switch (propertyCategory) {
        case PropertyCategory.interpolateFloat:
            returnType = "number"
            break;
        case PropertyCategory.stringValue:
            returnType = "string"
            break;
        case PropertyCategory.boolean:
            returnType = "boolean"
            break;
        case PropertyCategory.interpolateVector2:
            returnType = "vec2"
            break;
        case PropertyCategory.shape:
            returnType = "shape"
            break;
    }
    return returnType
}

function convertGraphValueToComponentValue(graphValue, valueType) {
    if(graphValue == null){
        return null
    }

    let graphValueType = graphValue.constructor.name

    if (graphValueType == "Float32Array") {
        if (valueType == "vec2") {
            return {x: graphValue[0], y: graphValue[1]}
        }
        if (valueType == "vec3") {
            return {x: graphValue[0], y: graphValue[1], z: graphValue[2]}
        }
    }

    if(graphValueType == "Point"){
        if (valueType == "vec2") {
            return {x: graphValue["x"], y: graphValue["y"]}
        }
    }

    return graphValue
}

export {getLiteGraphTypeFromPropertyType, getLiteGraphTypeFromPropertyCategory, convertGraphValueToComponentValue}