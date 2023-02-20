import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {huahuoEngine} from "../../EngineAPI";

import {IsValidWrappedObject} from "hhcommoncomponents";
import {internalProcessComponent} from "./AbstractVariableHandler";
import {capitalizeFirstLetter} from "../PropertySheetBuilder";

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

        let inserterName = "insert" + capitalizeFirstLetter(fieldName)

        internalProcessComponent(component, fieldName, {
            getter: () => {
                return new FieldShapeArrayIterable(component.rawObj.GetShapeArrayValue(fieldName))
            },
            inserter: (val: BaseShapeJS) => {
                if (!IsValidWrappedObject(component.rawObj.GetShapeArrayValue(fieldName))) {
                    component.rawObj.CreateShapeArrayValue(fieldName)
                }

                component.rawObj.GetShapeArrayValueForWrite(fieldName).InsertShape(val.getRawShape())

                if (component.baseShape)
                    component.baseShape.update(true)
                component.callHandlers(fieldName, null) // Is the val parameter really matters in this case?
            },
            deleter: (val) => {
                component.rawObj.GetShapeArrayValue(fieldName).DeleteShape(val)
                if (component.baseShape)
                    component.baseShape.update(true)

                component.callHandlers(fieldName, null) // Is the val parameter really matters in this case?
            }
        })
    }
}

let shapeArrayHandler = new ShapeArrayHandler()
export {shapeArrayHandler}