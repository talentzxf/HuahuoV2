import {PropertyType, getParameterNameAtIdx} from "hhcommoncomponents";
import {getEventParams} from "hhcommoncomponents/dist/src/EventBus/EventBus";

const graphActionSymbol = Symbol("graphAction")

class ActionParamDef {
    paramName: string
    paramType: PropertyType
    paramIndex: number
}

class ActionDef {
    actionName: string
    paramDefs: ActionParamDef[]
}

function getActions(target): object[] {
    let properties: object[] = Reflect.getMetadata(graphActionSymbol, target)
    if (!properties) {
        properties = new Array<ActionDef>()
        Reflect.defineMetadata(graphActionSymbol, properties, target)
    }

    return properties
}

function GraphAction() {
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor) {
        let actions = getActions(target)
        let actionDef: ActionDef = {
            actionName: propertyKey,
            paramDefs: []
        }
        actions.push(actionDef)
    }
}

const graphActionParamSymbol = Symbol("graphActionParam")

function getActionParams(target, functionName) {
    return Reflect.getMetadata(graphActionParamSymbol, target, functionName) || []
}

/**
 *
 * @param type
 * @param name There's no convenient way to get the parameter name from the JS runtime. So pass this parameter. If not set, we will try to get it from the function, but that might fail.
 * @constructor
 */
function ActionParam(type: PropertyType, name: string = null) {
    return function (target: Object, propertyKey: string | symbol, parameterIndex: number) {
        let existingParameters: ActionParamDef[] = getActionParams(target, propertyKey)

        let paramName = name || getParameterNameAtIdx(target[propertyKey], parameterIndex)
        existingParameters.push({
            paramName: paramName,
            paramType: type,
            paramIndex: parameterIndex
        })
        Reflect.defineMetadata(graphActionParamSymbol, existingParameters, target, propertyKey)
    }
}

abstract class AbstractGraphAction {
    getActionDefs(): Array<ActionDef> {
        return getActions(this) as Array<ActionDef>
    }

    protected constructor() {
        let actions = getActions(this)

        actions.forEach((actionDef: ActionDef) => {
            let actionParams = getActionParams(this, actionDef.actionName)
            actionDef.paramDefs = actionParams
        })
    }
}

export {AbstractGraphAction, ActionDef, ActionParam, GraphAction}