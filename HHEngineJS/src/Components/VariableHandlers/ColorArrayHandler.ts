import {capitalizeFirstLetter} from "../PropertySheetBuilder";
import {ColorStop} from "../ColorStop";
import {Logger} from "hhcommoncomponents";

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

class ColorStopArrayHandler{
    handleColorStopArrayEntry(component, propertyEntry) {
        let fieldName = propertyEntry["key"]
        component.rawObj.RegisterColorStopArrayValue(fieldName)

        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName) // Set is actually insert.
        let inserterName = "insert" + capitalizeFirstLetter(fieldName)
        let deleterName = "delete" + capitalizeFirstLetter(fieldName)
        let updaterName = "update" + capitalizeFirstLetter(fieldName)

        this[getterName] = function () {

            let colorStopArray = component.rawObj.GetColorStopArray(fieldName)

            if(colorStopArray.GetColorStopCount() < 2){ // Need at lease two counts.
                let currentColor = component.baseShape.paperShape.fillColor
                component.rawObj.AddColorStop(fieldName, 0.0, currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha)
                component.rawObj.AddColorStop(fieldName, 1.0, currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha)

                colorStopArray = component.rawObj.GetColorStopArray(fieldName)
            }

            return new ColorStopArrayIterable(colorStopArray)
        }.bind(this)

        // This is just alias of the insert funtion.
        this[setterName] = function (val) { // Return value is the identifier of the newly added colorStopEntry
            return this[inserterName](val)
        }.bind(this)

        this[inserterName] = function (val) {

            let retIndex = -1
            if(val instanceof ColorStop)
                retIndex = component.rawObj.AddColorStop(fieldName, val.value, val.r, val.g, val.b, val.a)
            else if(typeof(val) === "number")
                retIndex = component.rawObj.AddColorStop(fieldName, val)
            else
            {
                Logger.error("Why the val is not a ColorStop nor a number? It's actually:" + typeof(val))
                return -1
            }

            if (component.baseShape)
                component.baseShape.update(true)
            component.callHandlers(fieldName, val)

            return retIndex
        }.bind(this)

        this[deleterName] = function (val: ColorStop) {
            component.rawObj.DeleteColorStop(fieldName, val.identifier)
            if (component.baseShape)
                component.baseShape.update(true)

            component.callHandlers(fieldName, val) // Is the val parameter really matters in this case?
        }.bind(this)

        this[updaterName] = function(val: ColorStop){
            component.rawObj.UpdateColorStop(fieldName, val.identifier, val.value, val.r, val.g, val.b, val.a)
            if (component.baseShape)
                component.baseShape.update(true)

            component.callHandlers(fieldName, val)
        }

        // Remove the property and add setter/getter
        delete this[propertyEntry["key"]]

        // Add getter and setter
        Object.defineProperty(this, propertyEntry["key"], {
            get: function () {
                return this[getterName]()
            }
        })
    }
}

let colorStopArray = new ColorStopArrayHandler()
export {colorStopArray}