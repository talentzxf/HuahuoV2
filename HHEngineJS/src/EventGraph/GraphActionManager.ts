import {PropertyType, getParameterNameAtIdx} from "hhcommoncomponents";
const graphActionSymbol = Symbol("graphAction")

class ParamDef{
    paramName: string
    paramType: PropertyType
    paramIndex: number
}

class ActionDef {
    actionName
    paramTypes?: ParamDef[]
}

function getActions(target): object[] {
    let properties: object[] = Reflect.getMetadata(graphActionSymbol, target)
    if (!properties) {
        properties = new Array<ActionDef>()
        Reflect.defineMetadata(graphActionSymbol, properties, target)
    }

    return properties
}

function GraphAction(){
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor){
        let actions = getActions(target)
        let actionDef: ActionDef = {
            actionName: propertyKey
        }
        actions.push(actionDef)
    }
}

abstract class AbstractGraphAction {
    abstract getActionDefs(): ActionDef[]
}

class GraphActionManager {
}

export {GraphActionManager, AbstractGraphAction, ActionDef, GraphAction}