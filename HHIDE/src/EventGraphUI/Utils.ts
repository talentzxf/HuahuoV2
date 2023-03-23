import {PropertyType} from "hhcommoncomponents";

function getLiteGraphTypeFromPropertyType(propertyType: PropertyType){
    let returnType = ""
    switch(propertyType){
        case PropertyType.NUMBER:
            returnType = "number"
            break;
        case PropertyType.STRING:
            returnType = "string"
            break;
        case PropertyType.BOOLEAN:
            returnType = "boolean"
            break;
    }

    return returnType
}

export {getLiteGraphTypeFromPropertyType}