class EngineAPI{
    static inited = false

    static PendingInitFunctions = []

    static ExecuteAfterInited(func){
        if(EngineAPI.inited){
            return func();
        }

        EngineAPI.PendingInitFunctions.push(func);
    }

    static OnInit(){
        EngineAPI.inited = true

        EngineAPI.PendingInitFunctions.forEach(func=>{
            func();
        })
    }

    static getInstance(){
        if(EngineAPI.inited){
            return Module.HuaHuoEngine.prototype.getInstance()
        }else {
            console.log("Please wait while loading engine!")
            throw "Engine not inited!"
        }
    }
}


export {EngineAPI}