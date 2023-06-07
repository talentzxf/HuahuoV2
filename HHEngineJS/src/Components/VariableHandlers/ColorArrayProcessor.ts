import {capitalizeFirstLetter} from "hhcommoncomponents";
import {ColorStop} from "../ColorStop";
import {Logger} from "hhcommoncomponents";
import {internalProcessComponent} from "./AbstractVariableHandler";

class ColorStopArrayIterable {
    colorStopArray

    constructor(colorStopArray) {
        this.colorStopArray = colorStopArray
    }

    [Symbol.iterator]() {
        let curIdx = 0
        let _this = this
        const iterator = {
            next() {
                if (curIdx < _this.colorStopArray.GetColorStopCount()) {
                    let colorStop = new ColorStop(_this.colorStopArray.GetColorStop(curIdx++))
                    return {value: colorStop, done: false}
                }

                return {value: null, done: true}
            }
        }

        return iterator
    }
}

class ColorStopArrayProcessor {
    handleEntry(component, propertyEntry) {
        let fieldName = propertyEntry["key"]
        component.rawObj.RegisterColorStopArrayValue(fieldName)

        let inserterName = "insert" + capitalizeFirstLetter(fieldName)

        internalProcessComponent(component, fieldName, {
                getter: function () {
                    let colorStopArray = component.rawObj.GetColorStopArray(fieldName)

                    if (colorStopArray.GetColorStopCount() < 2) { // Need at lease two counts.
                        let currentColor = component.baseShape.paperShape.fillColor
                        component.rawObj.AddColorStop(fieldName, 0.0, currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha)
                        component.rawObj.AddColorStop(fieldName, 1.0, currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha)

                        colorStopArray = component.rawObj.GetColorStopArray(fieldName)
                    }

                    return new ColorStopArrayIterable(colorStopArray)
                },
                inserter: function (val) { // Return value is the identifier of the newly added colorStopEntry
                    let retIndex = -1
                    if (val instanceof ColorStop)
                        retIndex = component.rawObj.AddColorStop(fieldName, val.value, val.r, val.g, val.b, val.a)
                    else if (typeof (val) === "number")
                        retIndex = component.rawObj.AddColorStop(fieldName, val)
                    else {
                        Logger.error("Why the val is not a ColorStop nor a number? It's actually:" + typeof (val))
                        return -1
                    }

                    if (component.baseShape)
                        component.baseShape.update(true)
                    component.callHandlers(fieldName, val)

                    return retIndex
                },
                deleter: function (val: ColorStop) {
                    component.rawObj.DeleteColorStop(fieldName, val.identifier)
                    if (component.baseShape)
                        component.baseShape.update(true)

                    component.callHandlers(fieldName, val) // Is the val parameter really matters in this case?
                },
                updater: function (val: ColorStop) {
                    component.rawObj.UpdateColorStop(fieldName, val.identifier, val.value, val.r, val.g, val.b, val.a)
                    if (component.baseShape)
                        component.baseShape.update(true)

                    component.callHandlers(fieldName, val)
                },
                isVariableEqual: null // No need to check.
            }
        )
    }
}

let colorStopArrayHandler = new ColorStopArrayProcessor()
export {colorStopArrayHandler}