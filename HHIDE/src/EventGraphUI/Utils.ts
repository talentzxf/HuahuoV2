import {PropertyType, eventBus} from "hhcommoncomponents";

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
    }

    return returnType
}

function getEventCategoryMap(eventsFullNames): Map<string, Set<string>> {
    let eventCategoryMap: Map<string, Set<string>> = new Map // From Namespace to event name map.
    for (let eventFullName of eventsFullNames) {
        let eventNameInfo = eventBus.splitFullEventName(eventFullName)
        if (!eventCategoryMap.has(eventNameInfo.namespace)) {
            eventCategoryMap.set(eventNameInfo.namespace, new Set<string>())
        }
        eventCategoryMap.get(eventNameInfo.namespace).add(eventNameInfo.eventName)
    }

    return eventCategoryMap
}

export {getLiteGraphTypeFromPropertyType, getEventCategoryMap}