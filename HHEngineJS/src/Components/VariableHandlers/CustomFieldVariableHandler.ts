import {GroupComponent} from "../GroupComponent";
import {buildOperator, capitalizeFirstLetter} from "../PropertySheetBuilder";

class CustomFieldVariableHandler {

    // For cusome field, we don't have any knowledge other than name about it.
    // So, 1. replace the field with a hidden field.
    //     2. Setter is just call back all the handlers and set value to the hidden field.
    //     3. Getter is just return the hidden field.
    //     4. Caller should register event listener on the setter to do the real set actions if needed.
    handleEntry(component, propertyEntry) {
        let fieldName = propertyEntry["key"]

        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName) // Set is actually insert.

        let hiddenFieldName = "_" + fieldName

        component[setterName] = (val) => { // For custom field, only call the callbacks.
            component[hiddenFieldName] = val

            component.callHandlers(fieldName, val)
            if (component.baseShape) {
                component.baseShape.update(true)
                component.baseShape.callHandlers(fieldName, val)
            }
        }

        component[getterName] = () => {
            return component[hiddenFieldName]
        }

        delete component[fieldName]

        Object.defineProperty(component, fieldName, {
            get: function () {
                return component[getterName]()
            },
            set: function (val) {
                component[setterName](val)
            },
            configurable: true
        })
    }
}

let customFieldVariableHandler = new CustomFieldVariableHandler()
export {customFieldVariableHandler}