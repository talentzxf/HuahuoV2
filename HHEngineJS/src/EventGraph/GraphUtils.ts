import {PropertyType} from "hhcommoncomponents";

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

export {getLiteGraphTypeFromPropertyType}