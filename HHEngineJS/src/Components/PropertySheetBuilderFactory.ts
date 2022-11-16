import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {capitalizeFirstLetter, PropertyCategory, PropertyDef,} from "./PropertySheetBuilder";
import {PropertyType} from "hhcommoncomponents";

const propertyPrefix = "inspector.property."

class PropertySheetFactory{
    categoryTypeMap:Map<PropertyCategory, PropertyType> = new Map<PropertyCategory, PropertyType>()
    categoryElementTypeMap: Map<PropertyCategory, string> = new Map<PropertyCategory, string>()

    constructor(){
        this.categoryTypeMap.set(PropertyCategory.interpolateFloat, PropertyType.FLOAT)
        this.categoryElementTypeMap.set(PropertyCategory.interpolateFloat, "range")

        this.categoryTypeMap.set(PropertyCategory.interpolateColor, PropertyType.COLOR)

        this.categoryTypeMap.set(PropertyCategory.shapeArray, PropertyType.ARRAY)
        this.categoryElementTypeMap.set(PropertyCategory.shapeArray, PropertyType.REFERENCE)

        this.categoryTypeMap.set(PropertyCategory.colorStopArray, PropertyType.COLORSTOP)
    }

    createEntry(component, propertyMeta:PropertyDef, valueChangeHandler: ValueChangeHandler){
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

        let propertyType = this.categoryTypeMap.get(propertyMeta.type)
        propertyDef["type"] = propertyType
        if(this.categoryElementTypeMap.has(propertyMeta.type)){
            propertyDef["elementType"] = this.categoryElementTypeMap.get(propertyMeta.type)
        }

        return propertyDef
    }
}

let propertySheetFactory = new PropertySheetFactory()

export {propertySheetFactory}