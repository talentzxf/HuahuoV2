class EngineEventManager {
    private listeners:Map<string, Array<Function>> = new Map()

    registerEventListener(eventName:string, func:Function){
        if(!this.listeners.has(eventName)){
            this.listeners.set(eventName, new Array<Function>())
        }

        this.listeners.get(eventName).push(func);
    }

    dispatchEvent(eventName:string, ...params:any){
        let funcArray = this.listeners.get(eventName)
        if(funcArray){
            for(let func of funcArray){
                func(...params);
            }
        }
    }
}

let engineEventManager = new EngineEventManager()

export {engineEventManager}