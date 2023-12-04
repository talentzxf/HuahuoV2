import {ValueChangeHandler} from "hhenginejs";
import {PropertyCategory, PropertyDef} from "hhenginejs";
import {capitalizeFirstLetter, PropertyType} from "hhcommoncomponents"

const propertyPrefix = "inspector.property."

class PropertySheetFactory {
    categoryTypeMap: Map<PropertyCategory, PropertyType> = new Map<PropertyCategory, PropertyType>()
    categoryElementTypeMap: Map<PropertyCategory, string> = new Map<PropertyCategory, string>()

    constructor() {
        this.categoryTypeMap.set(PropertyCategory.interpolateFloat, PropertyType.NUMBER)
        this.categoryElementTypeMap.set(PropertyCategory.interpolateFloat, "range")

        this.categoryTypeMap.set(PropertyCategory.interpolateColor, PropertyType.COLOR)

        this.categoryTypeMap.set(PropertyCategory.shapeArray, PropertyType.ARRAY)
        this.categoryElementTypeMap.set(PropertyCategory.shapeArray, PropertyType.SHAPE)

        this.categoryTypeMap.set(PropertyCategory.shape, PropertyType.SHAPE)

        this.categoryTypeMap.set(PropertyCategory.colorStopArray, PropertyType.COLORSTOPARRAY)

        this.categoryTypeMap.set(PropertyCategory.interpolateVector2, PropertyType.VECTOR2)
        this.categoryTypeMap.set(PropertyCategory.interpolateVector3, PropertyType.VECTOR3)

        this.categoryTypeMap.set(PropertyCategory.keyframeArray, PropertyType.KEYFRAMES)

        this.categoryTypeMap.set(PropertyCategory.subcomponentArray, PropertyType.SUBCOMPONENTARRAY)
        this.categoryElementTypeMap.set(PropertyCategory.subcomponentArray, PropertyType.COMPONENT)

        this.categoryTypeMap.set(PropertyCategory.customField, PropertyType.CUSTOMFIELD)

        this.categoryTypeMap.set(PropertyCategory.stringValue, PropertyType.STRING)
    }

    getPropertyTypeFromPropertyCategory(propertyCategory: PropertyCategory){
        return this.categoryTypeMap.get(propertyCategory)
    }

    createEntryByNameAndCategory(propertyName, category: PropertyCategory, hide: boolean = false, singleLine: boolean = false) {
        let propertyDef = {
            key: propertyPrefix + propertyName,
        }

        if (hide) {
            propertyDef["hide"] = true
        }

        if (singleLine) {
            propertyDef["singleLine"] = true
        }

        let propertyType = this.categoryTypeMap.get(category)
        propertyDef["type"] = propertyType
        if (this.categoryElementTypeMap.has(category)) {
            propertyDef["elementType"] = this.categoryElementTypeMap.get(category)
        }

        return propertyDef
    }

    createEntry(component, propertyMeta: PropertyDef, valueChangeHandler: ValueChangeHandler) {
        let fieldName = propertyMeta["key"]

        let propertyDef = this.createEntryByNameAndCategory(propertyMeta["key"], propertyMeta.type,
            propertyMeta.hide, propertyMeta.singleLine)

        let isCustomField = propertyMeta.type == PropertyCategory.customField
        if (!isCustomField) {
            // Generate setter and getter
            let getterName = "get" + capitalizeFirstLetter(fieldName)
            let setterName = "set" + capitalizeFirstLetter(fieldName)
            let deleterName = "delete" + capitalizeFirstLetter(fieldName) // TODO: Code duplication with AbstractComponent
            let updateName = "update" + capitalizeFirstLetter(fieldName)

            let inserterName = "insert" + capitalizeFirstLetter(fieldName)

            propertyDef["getter"] = component[getterName].bind(component)

            if (component[setterName])
                propertyDef["setter"] = component[setterName].bind(component)


            propertyDef["config"] = propertyMeta.config

            if (component[updateName]) {
                propertyDef["updater"] = component[updateName].bind(component)
            }

            if (component[deleterName]) {
                propertyDef["deleter"] = component[deleterName].bind(component)
            }

            if (component[inserterName]) {
                propertyDef["inserter"] = component[inserterName].bind(component)
            }
        }

        propertyDef["config"] = propertyMeta.config
        propertyDef["registerValueChangeFunc"] = valueChangeHandler.registerValueChangeHandler(fieldName)
        propertyDef["unregisterValueChangeFunc"] = valueChangeHandler.unregisterValueChangeHandler(fieldName)

        if (propertyMeta.type == PropertyCategory.interpolateFloat) {
            if (!propertyDef.hasOwnProperty("config") || propertyDef["config"] == null) {
                propertyDef["config"] = {}
            }
            propertyDef["config"]["getKeyFrameCurve"] = () => {
                return component.getKeyFrameCurve(fieldName)
            }
        }

        if (propertyMeta.type == PropertyCategory.interpolateVector2) {
            if (!propertyDef.hasOwnProperty("config") || propertyDef["config"] == null) {
                propertyDef["config"] = {}
            }
            propertyDef["config"]["getKeyFrameCurves"] = () => {
                return component.getVector2KeyFrameCurves(fieldName)
            }
        }

        return propertyDef
    }
}

let propertySheetFactory = new PropertySheetFactory()

export {propertySheetFactory, propertyPrefix}