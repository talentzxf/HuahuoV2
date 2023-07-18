import {splitFullEventName} from "hhcommoncomponents";

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

export {getEventCategoryMap}