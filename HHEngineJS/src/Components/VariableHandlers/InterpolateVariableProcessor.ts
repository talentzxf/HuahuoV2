import {buildOperator} from "../PropertySheetBuilder";
import {internalProcessComponent} from "./AbstractVariableHandler";

class InterpolateVariableProcessor{
    handleEntry(component, propertyEntry) {
        let operator = buildOperator(propertyEntry.type, component.rawObj)
        operator.registerField(propertyEntry["key"], propertyEntry["initValue"])

        let fieldName = propertyEntry["key"]

        internalProcessComponent(component, fieldName, {
            getter: ()=>{
                return operator.getField(fieldName)
            },
            setter: (val)=>{
                operator.setField(fieldName, val)
            },
            isVariableEqual: (val1, val2)=>{
                return operator.isEqual(val1, val2)
            }
        })
    }
}

let interpolateVariableProcessor = new InterpolateVariableProcessor()
export {interpolateVariableProcessor}