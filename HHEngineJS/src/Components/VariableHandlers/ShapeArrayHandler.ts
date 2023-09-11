import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {huahuoEngine} from "../../EngineAPI";

import {IsValidWrappedObject, capitalizeFirstLetter} from "hhcommoncomponents";
import {internalProcessComponent} from "./AbstractVariableHandler";

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

class ShapeArrayHandler {
    handleEntry(component, propertyEntry) {
        let fieldName = propertyEntry["key"]
        component.shapeArrayFieldNames.add(fieldName)
        component.rawObj.RegisterShapeArrayValue(fieldName)

        internalProcessComponent(component, fieldName, {
            getter: () => {
                return new FieldShapeArrayIterable(component.rawObj.GetShapeArrayValue(fieldName))
            },
            updater: (idx, shape): boolean => {
                let returnValue = component.rawObj.GetShapeArrayValueForWrite(fieldName).UpdateShape(idx, shape == null ? 0 : shape.getRawObject())
                if (component.baseShape)
                    component.baseShape.update(true)
                component.callHandlers(fieldName, null) // Call handlers, so inspector can get the notification and update.

                return returnValue
            },
            contains: (val: BaseShapeJS) => {
                if (!IsValidWrappedObject(component.rawObj.GetShapeArrayValue(fieldName))) {
                    return false
                }

                return component.rawObj.GetShapeArrayValue(fieldName).ContainShape(val.getRawObject())
            },
            inserter: (val: BaseShapeJS) => {
                if (val == component.baseShape) // This will cause stack overflow.
                    return -1

                if (!IsValidWrappedObject(component.rawObj.GetShapeArrayValue(fieldName))) {
                    component.rawObj.CreateShapeArrayValue(fieldName)
                } else if (!propertyEntry.config.allowDuplication) {
                    if (val && component.rawObj.GetShapeArrayValue(fieldName).ContainShape(val.getRawObject())) {
                        return -1
                    }
                }

                let shapeIdx = component.rawObj.GetShapeArrayValueForWrite(fieldName).InsertShape(val == null ? 0 : val.getRawObject())

                if (component.baseShape)
                    component.baseShape.update(true)
                component.callHandlers(fieldName, null) // Call handlers, so inspector can get the notification and update.

                return shapeIdx
            },
            deleter: (val) => {
                component.rawObj.GetShapeArrayValue(fieldName).DeleteShape(val)
                if (component.baseShape)
                    component.baseShape.update(true)

                component.callHandlers(fieldName, null) // Call handlers, so inspector can get the notification and update.
            }
        })
    }
}

let shapeArrayHandler = new ShapeArrayHandler()
export {shapeArrayHandler}