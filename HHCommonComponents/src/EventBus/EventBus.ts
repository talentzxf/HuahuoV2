import {EventDef, getEvents} from "./EventEmitter";
import {PropertyType} from "../Properties/PropertySheet";
import {getParameterNameAtIdx} from "../Utils";

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

    public splitFullEventName(fullEventName: string){
        let splittedEventName = fullEventName.split(this.namespaceSeparator)
        return {
            namespace: splittedEventName[0],
            eventName: splittedEventName[1]
        }
    }

    public getAllGlobalEvents(){
        return this.globalEvents
    }

    public getAllEvents(){
        return this.eventHandlerIdMap.keys()
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

    removeEventHandler(namespace: string, evtName: string, handlerId: number){
        throw "Not Implemented!!!"
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

function GraphEvent(){
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor){
        // Register the event
        let events = getEvents(target)
        let eventDef: EventDef = {
            eventNameSpace: target.constructor.name,
            eventName: propertyKey,
            target: target
        }
        events.push(eventDef)

        let originalMethod = descriptor.value
        descriptor.value = function(...args:any[]){
            let executeResult = originalMethod.apply(this, args)
            eventBus.triggerEvent(target.constructor.name, propertyKey, args)
            return executeResult
        }
    }
}

const eventParameterSymbol = Symbol("eventParameter")

class EventParamDef {
    namespace: string
    eventName: string
    parameterName: string
    parameterType: PropertyType
    paramIndex: number
}

function getEventParams(target, functionName){
    return Reflect.getMetadata(eventParameterSymbol, target, functionName) || []
}

/**
 *
 * @param type
 * @param name There's no convenient way to get the parameter name from the JS runtime. So pass this parameter. If not set, we will try to get it from the function, but that might fail.
 * @constructor
 */
function EventParam(type: PropertyType, name: string = null){
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
        Reflect.defineMetadata(eventParameterSymbol, existingParameters, target, propertyKey)
    }
}


export {eventBus, HHEventBus, GraphEvent, EventParam, getEventParams}