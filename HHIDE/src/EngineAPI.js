class EngineAPI{
    static inited = false
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