import {Logger} from "hhcommoncomponents"

class EngineAPI{
    inited = false

    PendingInitFunctions = []

    constructor() {
        Logger.error("Creating Engine API!!!!")
    }

    ExecuteAfterInited(func){
        if(this.inited){
            return func();
        }

        this.PendingInitFunctions.push(func);
    }

    OnInit(){
        Module.HuaHuoEngine.prototype.InitEngine();
        this.inited = true
        Logger.error("Engine inited!!!!!")

        this.PendingInitFunctions.forEach(func=>{
            func();
        })
    }

    GetPersistentManager(){
        return Module.PersistentManager.prototype.GetPersistentManager()
    }
}

let huahuoEngine = new EngineAPI()
export {huahuoEngine}