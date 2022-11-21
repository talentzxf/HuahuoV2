import {buildOperator, capitalizeFirstLetter} from "../PropertySheetBuilder";
import {AbstractComponent} from "../AbstractComponent";

class InterpolateVariableHandler{
    handleInterpolateEntry(component, propertyEntry) {
        let _this = this

        let operator = buildOperator(propertyEntry.type, component.rawObj)

        operator.registerField(propertyEntry["key"], propertyEntry["initValue"])

        let fieldName = propertyEntry["key"]
        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName)

        component[setterName] = function (val: number) {
            let currentValue = this[fieldName]
            if (operator.isEqual(currentValue, val)) {
                return
            }

            component[fieldName] = val
            component.callHandlers(fieldName, val)
            if (component.baseShape) {
                component.baseShape.update(true)

                component.baseShape.callHandlers(fieldName, val)
            }
        }

        component[getterName] = function () {
            return operator.getField(fieldName)
        }

        // Remove the property and add setter/getter
        delete component[propertyEntry["key"]]

        // Add getter and setter
        Object.defineProperty(component, propertyEntry["key"], {
            get: function () {
                return component[getterName]()
            },
            set: function (val) {
                operator.setField(fieldName, val)
            }
        })

        // Store in cpp side on creation. Or else the information of the first frame might be lost.
        component[fieldName] = component[fieldName]
    }
}

let interpolateVariableHandler = new InterpolateVariableHandler()
export {interpolateVariableHandler}