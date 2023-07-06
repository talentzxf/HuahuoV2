import {PropertyType, splitFullEventName} from "hhcommoncomponents";

function getLiteGraphTypeFromPropertyType(propertyType: PropertyType) {
    let returnType = ""
    switch (propertyType) {
        case PropertyType.NUMBER:
            returnType = "number"
            break;
        case PropertyType.STRING:
            returnType = "string"
            break;
        case PropertyType.BOOLEAN:
            returnType = "boolean"
            break;
        case PropertyType.VECTOR2:
            returnType = "vec2"
            break;
    }

    return returnType
}

function getEventCategoryMap(eventsFullNames): Map<string, Set<object>> {
    let eventCategoryMap: Map<string, Set<object>> = new Map // From Namespace to event name map.

    eventsFullNames.forEach((eventSource, eventFullName)=>{
        let eventNameInfo = splitFullEventName(eventFullName)
        if (!eventCategoryMap.has(eventNameInfo.namespace)) {
            eventCategoryMap.set(eventNameInfo.namespace, new Set<object>())
        }

        eventCategoryMap.get(eventNameInfo.namespace).add({
            eventName: eventNameInfo.eventName,
            eventSource: eventSource
        })
    })

    return eventCategoryMap
}

export {getLiteGraphTypeFromPropertyType, getEventCategoryMap}