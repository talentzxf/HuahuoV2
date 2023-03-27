import {eventBus, getEventParams, HHEventBus} from "./EventBus";

const metaDataKey = Symbol("objectEvents")
import "reflect-metadata"

class EventDef{
    eventNameSpace: string
    eventName: string
    target: Object // target object.
}

function getEvents(target): object[] {
    let properties: object[] = Reflect.getMetadata(metaDataKey, target)
    if (!properties) {
        properties = new Array<EventDef>()
        Reflect.defineMetadata(metaDataKey, properties, target)
    }

    return properties
}
class EventEmitter{
    eventBus: HHEventBus = new HHEventBus()

    constructor(isGlobal = false) {
        let events = getEvents(this)
        events.forEach((eventDef: EventDef)=>{
            let eventParams = getEventParams(this, eventDef.eventName)

            let targetEventBus = isGlobal?eventBus: this.eventBus

            // The event is registered with the event emitter.
            targetEventBus.registerEvent(eventDef.eventNameSpace, eventDef.eventName, isGlobal, eventParams)
        })
    }

    getEvents(){
        return this.eventBus.getAllEvents()
    }
}

export {EventEmitter, getEvents, EventDef}