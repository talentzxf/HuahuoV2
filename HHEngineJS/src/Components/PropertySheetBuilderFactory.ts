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

        this.categoryTypeMap.set(PropertyCategory.colorStopArray, PropertyType.COLORSTOPARRAY)
    }

    createEntry(component, propertyMeta:PropertyDef, valueChangeHandler: ValueChangeHandler){
        let fieldName = propertyMeta["key"]
        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName)
        let deleterName = "delete" + capitalizeFirstLetter(fieldName) // TODO: Code duplication with AbstractComponent
        let updateName = "update" + capitalizeFirstLetter(fieldName)

        let propertyDef = {
            key: propertyPrefix + propertyMeta["key"],
            getter: component[getterName].bind(component),
            setter: component[setterName].bind(component),
            registerValueChangeFunc: valueChangeHandler.registerValueChangeHandler(fieldName),
            unregisterValueChagneFunc: valueChangeHandler.unregisterValueChangeHandler(fieldName),
            config: propertyMeta.config
        }

        if(component[updateName]){
            propertyDef["updater"] = component[updateName].bind(component)
            propertyDef["deleter"] = component[deleterName].bind(component)
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