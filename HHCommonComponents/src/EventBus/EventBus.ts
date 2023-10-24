import {EventDef, getEvents} from "./EventEmitter";
import {PropertyType} from "../Properties/PropertySheet";
import {getParameterNameAtIdx} from "../Utils";

class EventBusException {
    msg: string = "Unknoww Event Bus Exception"

    constructor(msg) {
        this.msg = msg
    }
}

const namespaceSeparator = ":"

function getFullEventName(namespace: string, evtName: string) {
    return namespace + namespaceSeparator + evtName
}

function splitFullEventName(fullEventName: string) {
    let splittedEventName = fullEventName.split(namespaceSeparator)
    return {
        namespace: splittedEventName[0],
        eventName: splittedEventName[1]
    }
}

class HHEventBus {

    maxHandlerId: number = 0
    handlerIdHandlerMap: Map<number, Function> = new Map()
    eventHandlerIdMap: Map<string, Set<number>> = new Map()
    eventHandlerParamArrayMap: Map<string, EventParamDef[]> = new Map()
    namespaceHandlerIdMap: Map<string, Set<number>> = new Map() // Whatever events happened in this namespace will trigger these wildcard handlers.

    public getAllEvents() {
        return this.eventHandlerIdMap.keys()
    }

    public registerEvent(namespace: string, evtName: string, params: EventParamDef[] = null) {
        if (evtName.indexOf(namespaceSeparator) != -1)
            throw new EventBusException("InvalidEventName:" + evtName)

        let fullEventName = getFullEventName(namespace, evtName)
        if (this.eventHandlerIdMap.has(fullEventName))
            return

        this.eventHandlerIdMap.set(fullEventName, new Set<number>())

        if (params && params.length > 0) {
            this.eventHandlerParamArrayMap.set(fullEventName, params)
        }
    }

    getEventParameters(fullEventName: string) {
        return this.eventHandlerParamArrayMap.get(fullEventName)
    }

    removeEventHandler(namespace: string, evtName: string, handlerId: number) {
        if (this.handlerIdHandlerMap.has(handlerId)) {
            this.handlerIdHandlerMap.delete(handlerId)

            this.eventHandlerIdMap.get(getFullEventName(namespace, evtName)).delete(handlerId)
        } else {
            console.warn("Trying to remove an nonexistence handlerId:" + handlerId)
        }
    }

    addEventHandler(namespace: string, evtName: string, handler: Function): number {

        let handlerIdArray = null
        if (evtName === "*") {
            if (!this.namespaceHandlerIdMap.has(namespace)) {
                this.namespaceHandlerIdMap.set(namespace, new Set<number>())
            }
            handlerIdArray = this.namespaceHandlerIdMap.get(namespace)
        } else {
            let fullEventName = getFullEventName(namespace, evtName)
            if (!this.eventHandlerIdMap.has(fullEventName)) {
                this.registerEvent(namespace, evtName)
            }

            handlerIdArray = this.eventHandlerIdMap.get(fullEventName)
        }

        if (handlerIdArray != null) {
            let handlerId = this.maxHandlerId++

            this.handlerIdHandlerMap.set(handlerId, handler)
            handlerIdArray.add(handlerId)
            return handlerId
        }
        return -1;
    }

    private _triggerEvent(handlerId, ...evtParams) {
        if (!this.handlerIdHandlerMap.has(handlerId)) {
            // throw new EventBusException("Can't find this handlerId:" + handlerId + " for event:" + fullEventName)
            return false
        }

        let func = this.handlerIdHandlerMap.get(handlerId)
        func(...evtParams)
        return true
    }

    triggerEvent(namespace: string, evtName: string, ...evtParams) {
        let fullEventName = getFullEventName(namespace, evtName)
        if (this.eventHandlerIdMap.has(fullEventName)) {
            let handlerIdArray = this.eventHandlerIdMap.get(fullEventName)
            for (let handlerId of handlerIdArray) {
                if (!this._triggerEvent(handlerId, ...evtParams)) {
                    console.error("Can't find this handlerId:" + handlerId + " for event:" + fullEventName)
                }
            }
        }

        if (this.namespaceHandlerIdMap.has(namespace)) {
            for (let handlerId of this.namespaceHandlerIdMap.get(namespace)) {
                if (!this._triggerEvent(handlerId, ...evtParams)) {
                    console.error("Can't find this handlerId:" + handlerId + " for event:" + fullEventName)
                }
            }
        }
    }
}

let eventBus = window["eventBus"]
if (!window["eventBus"]) {
    eventBus = new HHEventBus()
    window["eventBus"] = eventBus
}

function GraphEvent(isGlobal: boolean = false) {
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor) {

        let eventNameSpace = target.getEventEmitterName() || target.constructor.name

        // Register the event
        let events = getEvents(target)
        let eventDef: EventDef = {
            eventNameSpace: eventNameSpace,
            eventName: propertyKey,
            target: target,
            isGlobal: isGlobal
        }
        events.push(eventDef)

        let originalMethod = descriptor.value
        descriptor.value = function (...args: any[]) {
            let executeResult = originalMethod.apply(this, args)
            let eventNameSpace = target.getEventEmitterName() || target.constructor.name
            this.getEventBus().triggerEvent(eventNameSpace, propertyKey, args)

            if (isGlobal) // If this is a global event, trigger the event in the global namespace.
                eventBus.triggerEvent(eventNameSpace, propertyKey, args)
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

function getEventParams(target, functionName) {
    return Reflect.getMetadata(eventParameterSymbol, target, functionName) || []
}

/**
 *
 * @param type
 * @param name There's no convenient way to get the parameter name from the JS runtime. So pass this parameter. If not set, we will try to get it from the function, but that might fail.
 * @constructor
 */
function EventParam(type: PropertyType, name: string = null) {
    return function (target: Object, propertyKey: string | symbol, parameterIndex: number) {
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


export {eventBus, HHEventBus, GraphEvent, EventParam, getEventParams, getFullEventName, splitFullEventName}