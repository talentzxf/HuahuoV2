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

        this.categoryTypeMap.set(PropertyCategory.interpolateVector2, PropertyType.VECTOR2)
        this.categoryTypeMap.set(PropertyCategory.interpolateVector3, PropertyType.VECTOR3)

        this.categoryTypeMap.set(PropertyCategory.keyframeArray, PropertyType.KEYFRAMES)

        this.categoryTypeMap.set(PropertyCategory.subcomponentArray, PropertyType.SUBCOMPONENTARRAY)
    }

    createEntryByNameAndCategory(propertyName, category: PropertyCategory) {
        let propertyDef = {
            key: propertyPrefix + propertyName,
        }
        let propertyType = this.categoryTypeMap.get(category)
        propertyDef["type"] = propertyType
        if (this.categoryElementTypeMap.has(category)) {
            propertyDef["elementType"] = this.categoryElementTypeMap.get(category)
        }

        return propertyDef
    }


    createEntry(component, propertyMeta:PropertyDef, valueChangeHandler: ValueChangeHandler){
        let fieldName = propertyMeta["key"]
        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName)
        let deleterName = "delete" + capitalizeFirstLetter(fieldName) // TODO: Code duplication with AbstractComponent
        let updateName = "update" + capitalizeFirstLetter(fieldName)

        let propertyDef = this.createEntryByNameAndCategory(propertyMeta["key"], propertyMeta.type)

        propertyDef["getter"] = component[getterName].bind(component),
        propertyDef["setter"] = component[setterName].bind(component),
        propertyDef["registerValueChangeFunc"] = valueChangeHandler.registerValueChangeHandler(fieldName),
        propertyDef["unregisterValueChagneFunc"] = valueChangeHandler.unregisterValueChangeHandler(fieldName),
        propertyDef["config"] = propertyMeta.config

        if(component[updateName]){
            propertyDef["updater"] = component[updateName].bind(component)
            propertyDef["deleter"] = component[deleterName].bind(component)
        }

        return propertyDef
    }
}

let propertySheetFactory = new PropertySheetFactory()

export {propertySheetFactory, propertyPrefix}