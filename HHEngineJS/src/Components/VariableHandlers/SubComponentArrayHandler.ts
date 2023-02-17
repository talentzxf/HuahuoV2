import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {huahuoEngine} from "../../EngineAPI";

import {IsValidWrappedObject} from "hhcommoncomponents";
import {internalProcessComponent} from "./AbstractVariableHandler";
import {capitalizeFirstLetter} from "../PropertySheetBuilder";
import {AbstractComponent} from "../AbstractComponent";
import {clzObjectFactory} from "../../CppClassObjectFactory";
import {GroupComponent} from "../GroupComponent";

class FieldSubComponentArrayIterable {
    parentComponent: GroupComponent
    constructor(parentComponent: GroupComponent) {
        this.parentComponent = parentComponent
    }

    [Symbol.iterator]() {
        let curIdx = 0
        let _this = this
        const iterator = {
            next() {
                if (curIdx < _this.parentComponent.rawObj.GetSubComponentCount()) {
                    let componentRawObj = _this.parentComponent.rawObj.GetSubComponentByIdx(curIdx)
                    let targetComponent = _this.parentComponent.getComponentByRawObj(componentRawObj)
                    return {value: targetComponent, done: false}
                }

                return {value: null, done: true}
            }
        }

        return iterator
    }
}

class SubComponentArrayHandler {
    handleEntry(component: GroupComponent, propertyEntry) {
        let fieldName = propertyEntry["key"]
        component.rawObj.RegisterSubcomponentArray(fieldName)
        let rawSubComponent = component.rawObj.GetSubComponentByName(fieldName)

        // Recreate the subcomponent
        let subComponentConstructor = clzObjectFactory.GetClassConstructor(rawSubComponent.GetTypeName())
        let groupComponent = new subComponentConstructor(rawSubComponent)
        component.addSubComponent(groupComponent)

        let inserterName = "insert" + capitalizeFirstLetter(fieldName)

        internalProcessComponent(component, fieldName, {
            getter: () => {
                return new FieldSubComponentArrayIterable(component)
            },
            inserter: (val: AbstractComponent) => {

            },
            deleter: (val) => {
            },
            setter: (val) => {
                component[inserterName](val)
            } // Setter is just alias of inserter.
        })
    }
}

let subComponentArrayHandler = new SubComponentArrayHandler()
export {subComponentArrayHandler}