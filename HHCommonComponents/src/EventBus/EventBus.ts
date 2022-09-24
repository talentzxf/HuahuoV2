class EventBusException{
    msg:string = "Unknoww Event Bus Exception"
    constructor(msg) {
        this.msg = msg
    }
}

class HHEventBus{
    maxHandlerId: number = 0
    handlerIdHandlerMap: Map<number, Function> = new Map()
    eventHandlerIdMap: Map<string, Set<number>> = new Map()

    getFullEventName(evtName:string, namespace:string){
        return namespace + "." + evtName
    }

    registerEvent(evtName:string, namespace: string = "global"){
        if(evtName.indexOf(".") != -1)
            throw new EventBusException("InvalidEventName:" + evtName)

        let fullEventName = this.getFullEventName(evtName, namespace)
        if(this.eventHandlerIdMap.has(fullEventName))
            throw new EventBusException("Event has already been registered, can't register twice")

        this.eventHandlerIdMap.set(fullEventName, new Set<number>())
    }

    addEventHandler(evtName: string, namespace: string = "global", handler: Function): number{
        let fullEventName = this.getFullEventName(evtName, namespace)
        if(!this.eventHandlerIdMap.has(fullEventName)){
            throw new EventBusException("event has not been registered!")
        }

        let handlerIdArray = this.eventHandlerIdMap.get(fullEventName)
        let handlerId = this.maxHandlerId++
        this.handlerIdHandlerMap.set(handlerId, handler)

        handlerIdArray.add(handlerId)
        return handlerId
    }

    triggerEvent(evtName: string, namespace: string = "global", ...evtParams){
        let fullEventName = this.getFullEventName(evtName, namespace)
        if(!this.eventHandlerIdMap.has(fullEventName)){
            throw new EventBusException("event has not been registered!")
        }

        let handlerIdArray = this.eventHandlerIdMap.get(fullEventName)
        for(let handlerId of handlerIdArray){
            if(!this.handlerIdHandlerMap.has(handlerId)){
                throw new EventBusException("Can't find this handlerId:" + handlerId + " for event:" + fullEventName)
            }

            let func = this.handlerIdHandlerMap.get(handlerId)
            func(...evtParams)
        }
    }
}

let eventBus = window["eventBus"]
if(!window["eventBus"]){
    eventBus = new HHEventBus()
    window["eventBus"] = eventBus
}

export {eventBus}