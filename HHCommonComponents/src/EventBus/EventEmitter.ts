import {eventBus, eventOutParameter, getEventParams} from "./EventBus";

const metaDataKey = Symbol("objectEvents")
import "reflect-metadata"

class EventDef{
    eventNameSpace: string
    eventName: string
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
    constructor(isGlobal = false) {
        let events = getEvents(this)
        events.forEach((eventDef: EventDef)=>{
            let eventParams = getEventParams(this, eventDef.eventName)

            eventBus.registerEvent(eventDef.eventNameSpace, eventDef.eventName, isGlobal, eventParams)
        })
    }
}

export {EventEmitter, getEvents, EventDef}