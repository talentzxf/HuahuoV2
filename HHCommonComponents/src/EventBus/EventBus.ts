import {EventDef, getEvents} from "./EventEmitter";
import {PropertyType} from "../Properties/PropertySheet";

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
    eventHandlerParamArrayMap: Map<string, EventParamDef[]> = new Map()
    namespaceHandlerIdMap: Map<string, Set<number>> = new Map()
    globalEvents: Array<string> = new Array()

    private getFullEventName(namespace:string, evtName:string){
        return namespace + this.namespaceSeparator + evtName
    }

    public getAllGlobalEvents(){
        return this.globalEvents
    }

    public registerEvent(namespace: string, evtName:string, isGlobal: boolean = false, params: EventParamDef[] = null){
        if(evtName.indexOf(this.namespaceSeparator) != -1)
            throw new EventBusException("InvalidEventName:" + evtName)

        let fullEventName = this.getFullEventName(namespace, evtName)
        if(this.eventHandlerIdMap.has(fullEventName))
            return

        if(isGlobal)
            this.globalEvents.push(fullEventName)

        this.eventHandlerIdMap.set(fullEventName, new Set<number>())

        if(params && params.length > 0){
            this.eventHandlerParamArrayMap.set(fullEventName, params)
        }
    }

    getEventParameters(fullEventName: string){
        return this.eventHandlerParamArrayMap.get(fullEventName)
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

            return originalMethod.apply(this, args)
        }
    }
}

const eventOutParameter = Symbol("eventOutParameter")

class EventParamDef {
    namespace: string
    eventName: string
    parameterName: string
    parameterType: PropertyType
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
    return Reflect.getMetadata(eventOutParameter, target, functionName) || []
}

/**
 *
 * @param type
 * @param name There's no convenient way to get the parameter name from the JS runtime. So pass this parameter. If not set, we will try to get it from the function, but that might fail.
 * @constructor
 */
function EventOut(type: PropertyType, name: string = null){
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