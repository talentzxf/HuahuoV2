import {capitalizeFirstLetter} from "../PropertySheetBuilder";
import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {huahuoEngine} from "../../EngineAPI";

import {IsValidWrappedObject, PropertyConfig, PropertyType} from "hhcommoncomponents";

class FieldShapeArrayIterable {
    fieldShapeArray // Store the cpp side array.
    constructor(fieldShapeArray) {
        this.fieldShapeArray = fieldShapeArray
    }

    [Symbol.iterator]() {
        let curIdx = 0
        let _this = this
        const iterator = {
            next() {
                if (curIdx < _this.fieldShapeArray.GetShapeCount()) {
                    let targetShape = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(_this.fieldShapeArray.GetShape(curIdx++), true)
                    return {value: targetShape, done: false}
                }

                return {value: null, done: true}
            }
        }

        return iterator
    }
}

class ShapeArrayHandler{
    handleShapeArrayEntry(component, propertyEntry) {
        let fieldName = propertyEntry["key"]
        component.shapeArrayFieldNames.add(fieldName)
        component.rawObj.RegisterShapeArrayValue(fieldName)

        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName) // Set is actually insert.
        let inserterName = "insert" + capitalizeFirstLetter(fieldName)
        let deleterName = "delete" + capitalizeFirstLetter(fieldName)

        component[getterName] = function () {
            return new FieldShapeArrayIterable(component.rawObj.GetShapeArrayValue(fieldName))
        }.bind(component)

        // This is just alias of the insert funtion.
        component[setterName] = function (val) {
            component[inserterName](val)
        }.bind(component)

        component[inserterName] = function (val: BaseShapeJS) {
            if (!IsValidWrappedObject(component.rawObj.GetShapeArrayValue(fieldName))) {
                component.rawObj.CreateShapeArrayValue(fieldName)
            }

            component.rawObj.GetShapeArrayValueForWrite(fieldName).InsertShape(val.getRawShape())

            if (component.baseShape)
                component.baseShape.update(true)
            component.callHandlers(fieldName, null) // Is the val parameter really matters in this case?
        }.bind(component)

        component[deleterName] = function (val) {
            component.rawObj.GetShapeArrayValue(fieldName).DeleteShape(val)
            if (component.baseShape)
                component.baseShape.update(true)

            component.callHandlers(fieldName, null) // Is the val parameter really matters in this case?
        }.bind(component)

        // Remove the property and add setter/getter
        delete component[propertyEntry["key"]]

        // Add getter and setter
        Object.defineProperty(component, propertyEntry["key"], {
            get: function () {
                return component[getterName]()
            }
        })
    }
}

let shapeArrayHandler = new ShapeArrayHandler()
export {shapeArrayHandler}