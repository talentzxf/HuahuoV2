import {eventBus, getEventParams, HHEventBus} from "./EventBus";

const metaDataKey = Symbol("objectEvents")
import "reflect-metadata"

class EventDef{
    eventNameSpace: string
    eventName: string
    target: Object // target object.
    isGlobal: boolean
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

    constructor() {
        let events = getEvents(this)
        events.forEach((eventDef: EventDef)=>{
            let eventParams = getEventParams(this, eventDef.eventName)

            // The event is registered with the event emitter.
            this.eventBus.registerEvent(eventDef.eventNameSpace, eventDef.eventName, eventParams)

            if(eventDef.isGlobal){
                eventBus.registerEvent(eventDef.eventNameSpace, eventDef.eventName, eventParams)
            }
        })
    }

    getEventBus(){
        return this.eventBus
    }

    getEvents(){
        return this.eventBus.getAllEvents()
    }
}

export {EventEmitter, getEvents, EventDef}