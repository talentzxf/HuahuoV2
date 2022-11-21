import {AbstractComponent} from "../AbstractComponent";
import {capitalizeFirstLetter} from "../PropertySheetBuilder";

class VariableHandlerConfig {
    setter?: Function
    getter?: Function
    updater?: Function
    inserter?: Function
    deleter?: Function
}

function internalProcessComponent(component: AbstractComponent, fieldName: string, config: VariableHandlerConfig) {
    let getterName = "get" + capitalizeFirstLetter(fieldName)
    if (config.getter) {
        component[getterName] = config.getter.bind(component)
    }

    let setterName = "set" + capitalizeFirstLetter(fieldName) // Set is actually insert.
    if (config.setter) {
        component[setterName] = config.setter.bind(component)
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
            }
        })
    } else if (config.getter) {
        // Add getter and setter
        Object.defineProperty(component, fieldName, {
            get: function () {
                return component[getterName]()
            },
            set: function (val) {
                component[setterName](val)
            }
        })
    }
}

export {internalProcessComponent}