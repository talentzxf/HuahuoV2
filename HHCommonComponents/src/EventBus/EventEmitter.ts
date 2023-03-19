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
    constructor() {
        let events = getEvents(this)
        events.forEach((eventDef: EventDef)=>{
            eventBus.registerEvent(eventDef.eventNameSpace, eventDef.eventName)

            let eventParams = getEventParams(this, eventDef.eventName)
        })
    }
}

export {EventEmitter, getEvents, EventDef}