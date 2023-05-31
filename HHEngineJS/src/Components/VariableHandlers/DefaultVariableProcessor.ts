import {buildOperator} from "../PropertySheetBuilder";
import {internalProcessComponent} from "./AbstractVariableHandler";

class DefaultVariableProcessor{
    handleEntry(component, propertyEntry) {
        let operator = buildOperator(propertyEntry.type, component.rawObj)
        operator.registerField(propertyEntry["key"], propertyEntry["initValue"])

        let fieldName = propertyEntry["key"]

        delete this[fieldName] // Undefine the property

        internalProcessComponent(component, fieldName, {
            getter: ()=>{
                return operator.getField(fieldName)
            },
            setter: (val)=>{
                return operator.setField(fieldName, val)
            },
            isVariableEqual: (val1, val2)=>{
                return operator.isEqual(val1, val2)
            }
        })
    }
}

let defaultVariableProcessor = new DefaultVariableProcessor()
export {defaultVariableProcessor}