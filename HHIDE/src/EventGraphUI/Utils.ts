import {splitFullEventName} from "hhcommoncomponents";
import {NodeTargetType} from "hhenginejs"

function getEventCategoryMap(eventsTypes, eventAdditionalInfos): Map<string, Set<object>> {
    let eventCategoryMap: Map<string, Set<object>> = new Map // From Namespace to event name map.

    eventsTypes.forEach((eventType: NodeTargetType, eventFullName) => {
        let eventNameInfo = splitFullEventName(eventFullName)
        if (!eventCategoryMap.has(eventNameInfo.namespace)) {
            eventCategoryMap.set(eventNameInfo.namespace, new Set<object>())
        }

        eventCategoryMap.get(eventNameInfo.namespace).add({
            eventName: eventNameInfo.eventName,
            additionalInfo: eventAdditionalInfos.get(eventFullName),
            eventType: eventsTypes.get(eventFullName)
        })
    })

    return eventCategoryMap
}

export {getEventCategoryMap}