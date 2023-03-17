class EventBusException{
    msg:string = "Unknoww Event Bus Exception"
    constructor(msg) {
        this.msg = msg
    }
}

class HHEventBus{
    private namespaceSeparator = ":"
    maxHandlerId: number = 0
    handlerIdHandlerMap: Map<number, Function> = new Map()
    eventHandlerIdMap: Map<string, Set<number>> = new Map()

    private getFullEventName(namespace:string, evtName:string){
        return namespace + this.namespaceSeparator + evtName
    }

    private registerEvent(namespace: string, evtName:string){
        if(evtName.indexOf(this.namespaceSeparator) != -1)
            throw new EventBusException("InvalidEventName:" + evtName)

        let fullEventName = this.getFullEventName(namespace, evtName)
        if(this.eventHandlerIdMap.has(fullEventName))
            return

        this.eventHandlerIdMap.set(fullEventName, new Set<number>())
    }

    addEventHandler(namespace: string, evtName: string, handler: Function): number{
        let fullEventName = this.getFullEventName(namespace, evtName)
        if(!this.eventHandlerIdMap.has(fullEventName)){
            this.registerEvent(namespace, evtName)
        }

        let handlerIdArray = this.eventHandlerIdMap.get(fullEventName)
        let handlerId = this.maxHandlerId++
        this.handlerIdHandlerMap.set(handlerId, handler)

        handlerIdArray.add(handlerId)
        return handlerId
    }

    triggerEvent(namespace: string, evtName: string, ...evtParams){
        let fullEventName = this.getFullEventName(namespace, evtName)
        if(!this.eventHandlerIdMap.has(fullEventName)){
            return
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

function TriggerEvent(){
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor){
        let originalMethod = descriptor.value
        descriptor.value = function(...args:any[]){
            eventBus.triggerEvent(target.constructor.name, propertyKey, args)

            return originalMethod.app(this, args)
        }
    }
}


export {eventBus, TriggerEvent}