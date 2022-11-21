import {AbstractComponent} from "../AbstractComponent";
import {capitalizeFirstLetter} from "../PropertySheetBuilder";

class VariableHandlerConfig {
    setter?: Function
    getter: Function // Getter is a must!
    updater?: Function
    inserter?: Function
    deleter?: Function

    isVariableEqual?:Function = function(val1, val2){
        return val1 == val2
    }
}

function internalProcessComponent(component: AbstractComponent, fieldName: string, config: VariableHandlerConfig) {
    let getterName = "get" + capitalizeFirstLetter(fieldName)
    if (config.getter) {
        component[getterName] = config.getter.bind(component)
    }

    let setterName = "set" + capitalizeFirstLetter(fieldName) // Set is actually insert.
    if (config.setter) { // Call setter function will trigger events.
        component[setterName] = (val)=>{
            let currentValue = component[fieldName]
            if (config.isVariableEqual && config.isVariableEqual(currentValue, val)) {
                return
            }

            config.setter(val)
            component.callHandlers(fieldName, val)
            if (component.baseShape) {
                component.baseShape.update(true)

                component.baseShape.callHandlers(fieldName, val)
            }
        }
    }

    if (config.deleter) {
        let deleterName = "delete" + capitalizeFirstLetter(fieldName)
        component[deleterName] = config.deleter.bind(component)
    }

    if (config.inserter) {
        let inserterName = "insert" + capitalizeFirstLetter(fieldName)
        component[inserterName] = config.inserter.bind(component)
    }

    if (config.updater) {
        let updaterName = "update" + capitalizeFirstLetter(fieldName)
        component[updaterName] = config.updater.bind(component)
    }

    // Remove the property and add setter/getter
    delete component[fieldName]

    if (config.getter && config.setter) {
        // Add getter and setter
        Object.defineProperty(component, fieldName, {
            get: function () {
                return component[getterName]()
            },
            set: function (val) { // Direct set of the variable won't trigger events.
                config.setter(val)
            }
        })
    } else if (config.getter) {
        // Add getter only
        Object.defineProperty(component, fieldName, {
            get: function () {
                return component[getterName]()
            }
        })
    }
}

export {internalProcessComponent}