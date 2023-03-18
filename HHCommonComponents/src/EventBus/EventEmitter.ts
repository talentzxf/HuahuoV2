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

    }
}

export {EventEmitter, getEvents, EventDef}