import {Logger} from "hhcommoncomponents"
import {Player} from "./Player/Player";

class EngineAPI{
    inited = false

    PendingInitFunctions = []

    storeIdElementShapeMap = {}

    cppEngine = null

    activePlayer = null

    constructor() {
        Logger.info("Creating Engine API!!!!")
    }

    setActivePlayer(activePlayer){
        this.activePlayer = activePlayer
    }

    getActivePlayer(){
        return this.activePlayer
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
        if(this.inited)
            return;

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

    GetCurrentStoreId(){
        return this.GetCurrentStore().GetStoreId()
    }

    getStoreMaxFrames(storeId){
        return this.GetDefaultObjectStoreManager().GetStoreById(storeId).GetMaxFrameId()
    }

    GetCurrentLayer(){
        let layer = this.GetCurrentStore().GetCurrentLayer();
        if(!layer.addShape){
            layer.addShape = (shape)=>{
                shape.update()
                layer.AddShapeInternal(shape.getRawShape())
                shape.isPermanent = true

                if(this.activePlayer){
                    this.activePlayer.getLayerShapes(layer).set(shape.getRawShape(), shape)
                }

                Logger.debug("Currently there're:" + layer.GetShapeCount() + " shapes in the layer.")
            }
        }
        return layer
    }

    RegisterElementShape(storeId, element){
        this.storeIdElementShapeMap[storeId] = element
    }

    GetElementShapeByStoreId(storeId){
        return this.storeIdElementShapeMap[storeId]
    }
}

let huahuoEngine = window.huahuoEngine
if(!window.huahuoEngine){
    huahuoEngine = new EngineAPI()
    window.huahuoEngine = huahuoEngine
}

export {huahuoEngine}