import {buildOperator} from "../PropertySheetBuilder";
import {internalProcessComponent} from "./AbstractVariableHandler";
import {AbstractComponent} from "../AbstractComponent";

class DefaultVariableProcessor{
    handleEntry(component: AbstractComponent, propertyEntry) {
        let operator = buildOperator(propertyEntry.type, component.rawObj)
        operator.registerField(propertyEntry["key"], propertyEntry["initValue"])

        let fieldName = propertyEntry["key"]

        delete component[fieldName] // Undefine the property

        internalProcessComponent(component, fieldName, {
            getter: ()=>{
                if(component.getActor().hasField(fieldName))
                    return component.getActor().getField(fieldName)
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