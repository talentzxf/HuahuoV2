import {Logger} from "hhcommoncomponents"

class EngineAPI{
    inited = false

    PendingInitFunctions = []

    cppEngine = null

    constructor() {
        Logger.info("Creating Engine API!!!!")
    }

    ExecuteAfterInited(func){
        if(this.inited){
            return func();
        }

        this.PendingInitFunctions.push(func);
    }

    GetInstance(){
        return this.cppEngine;
    }

    OnInit(){
        Module.HuaHuoEngine.prototype.InitEngine();
        this.cppEngine = Module.HuaHuoEngine.prototype.GetInstance();
        this.inited = true
        Logger.error("Engine inited!!!!!")

        this.PendingInitFunctions.forEach(func=>{
            func();
        })
    }

    CreateShape(shapeName){
        return this.cppEngine.CreateShape(shapeName)
    }

    GetPersistentManager(){
        return Module.PersistentManager.prototype.GetPersistentManager()
    }

    GetDefaultObjectStoreManager(){
        return Module.ObjectStoreManager.prototype.GetDefaultObjectStoreManager()
    }

    GetCurrentStore(){
        return this.GetDefaultObjectStoreManager().GetCurrentStore()
    }

    GetCurrentLayer(){
        let layer = this.GetCurrentStore().GetCurrentLayer();
        if(!layer.addShape){
            layer.addShape = (shape)=>{
                layer.AddShapeInternal(shape.getRawShape())
                Logger.debug("Currently there're:" + layer.GetShapeCount() + " shapes in the layer.")
            }
        }
        return layer
    }
}

let huahuoEngine = new EngineAPI()
export {huahuoEngine}