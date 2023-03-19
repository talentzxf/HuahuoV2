import {EventDef, getEvents} from "./EventEmitter";
import {EventParameterType} from "./EventParameterType";

class EventBusException{
    msg:string = "Unknoww Event Bus Exception"
    constructor(msg) {
        this.msg = msg
    }
}

class HHEventBus{
    private namespaceSeparator = ":"
    maxHandlerId: number = 0
    handlerIdHandlerMap: Map<number, Function> = new Map()
    eventHandlerIdMap: Map<string, Set<number>> = new Map()
    namespaceHandlerIdMap: Map<string, Set<number>> = new Map()


    private getFullEventName(namespace:string, evtName:string){
        return namespace + this.namespaceSeparator + evtName
    }

    public registerEvent(namespace: string, evtName:string){
        if(evtName.indexOf(this.namespaceSeparator) != -1)
            throw new EventBusException("InvalidEventName:" + evtName)

        let fullEventName = this.getFullEventName(namespace, evtName)
        if(this.eventHandlerIdMap.has(fullEventName))
            return

        this.eventHandlerIdMap.set(fullEventName, new Set<number>())
    }

    getEvents(namespaces: string[]){
        let eventNames = []
        let _this = this
        namespaces.forEach((namespace)=>{
            _this.eventHandlerIdMap
        })
    }

    addEventHandler(namespace: string, evtName: string, handler: Function): number{
        let fullEventName = this.getFullEventName(namespace, evtName)
        if(!this.eventHandlerIdMap.has(fullEventName)){
            this.registerEvent(namespace, evtName)
        }

        let handlerIdArray = this.eventHandlerIdMap.get(fullEventName)
        let handlerId = this.maxHandlerId++
        this.handlerIdHandlerMap.set(handlerId, handler)

        handlerIdArray.add(handlerId)
        return handlerId
    }

    triggerEvent(namespace: string, evtName: string, ...evtParams){
        let fullEventName = this.getFullEventName(namespace, evtName)
        if(!this.eventHandlerIdMap.has(fullEventName)){
            return
        }

        let handlerIdArray = this.eventHandlerIdMap.get(fullEventName)
        for(let handlerId of handlerIdArray){
            if(!this.handlerIdHandlerMap.has(handlerId)){
                throw new EventBusException("Can't find this handlerId:" + handlerId + " for event:" + fullEventName)
            }

            let func = this.handlerIdHandlerMap.get(handlerId)
            func(...evtParams)
        }
    }
}

let eventBus = window["eventBus"]
if(!window["eventBus"]){
    eventBus = new HHEventBus()
    window["eventBus"] = eventBus
}

function TriggerEvent(){
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor){
        // Register the event
        let events = getEvents(target)
        let eventDef: EventDef = {
            eventNameSpace: target.constructor.name,
            eventName: propertyKey
        }
        events.push(eventDef)

        let originalMethod = descriptor.value
        descriptor.value = function(...args:any[]){
            eventBus.triggerEvent(target.constructor.name, propertyKey, args)

            return originalMethod.app(this, args)
        }
    }
}

const eventOutParameter = Symbol("eventOutParameter")

class EventParamDef {
    namespace: string
    eventName: string
    parameterName: string
    parameterType: EventParameterType
    paramIndex: number
}

function getParameterNameAtIdx(func: Function, paramIdx: number){
    const funcStr = func.toString();
    const paramNames = funcStr.slice(funcStr.indexOf('(') + 1, funcStr.indexOf(')')).match(/([^\s,]+)/g) || []
    if(paramNames.length < paramIdx){
        return null
    }

    return paramNames[paramIdx]
}

function getEventParams(target, functionName){
    return Reflect.getOwnMetadata(eventOutParameter, target, functionName) || []
}

/**
 *
 * @param type
 * @param name There's no convenient way to get the parameter name from the JS runtime. So pass this parameter. If not set, we will try to get it from the function, but that might fail.
 * @constructor
 */
function EventOut(type: EventParameterType, name: string = null){
    return function(target: Object, propertyKey: string | symbol, parameterIndex: number){
        let existingParameters: EventParamDef[] = getEventParams(target, propertyKey)

        let paramName = name || getParameterNameAtIdx(target[propertyKey], parameterIndex)
        existingParameters.push({
            namespace: target.constructor.name,
            eventName: propertyKey as string,
            paramIndex: parameterIndex,
            parameterName: paramName,
            parameterType: type
        })
        Reflect.defineMetadata(eventOutParameter, existingParameters, target, propertyKey)
    }
}


export {eventBus, TriggerEvent, EventOut, eventOutParameter, getEventParams}