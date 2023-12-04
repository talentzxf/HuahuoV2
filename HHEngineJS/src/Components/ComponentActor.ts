import {getProperties} from "./AbstractComponent";
import {capitalizeFirstLetter} from "hhcommoncomponents";


class ComponentActor {
    fieldValueMap: Map<string, object> = new Map()

    targetComponent = null

    constructor(targetComponent) {
        this.targetComponent = targetComponent

        // Set up all setters and getters
        let properties = getProperties(this.targetComponent)
        for (let propertyDef of properties) {
            if (propertyDef.hide)
                continue

            let getterName = "get" + capitalizeFirstLetter(propertyDef.key)
            let setterName = "set" + capitalizeFirstLetter(propertyDef.key)
            this[getterName] = () => {
                this.targetComponent[getterName]()
            }

            this[setterName] = (val) => {
                this.setField(propertyDef.key, val)
            }
        }
    }

    setField(fieldName: string, value: object) {
        this.fieldValueMap.set(fieldName, value)
    }

    hasField(fieldName: string) {
        return this.fieldValueMap.has(fieldName)
    }

    getField(fieldName: string) {
        if (!this.fieldValueMap.has(fieldName))
            return null

        return this.fieldValueMap.get(fieldName)
    }

    reset() {
        this.fieldValueMap = new Map()
    }
}

export {ComponentActor}