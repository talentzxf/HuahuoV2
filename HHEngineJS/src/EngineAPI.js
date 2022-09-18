import {Logger} from "hhcommoncomponents"
import {Player} from "./Player/Player";
import {engineEventManager} from "./EngineEvents/EngineEventManager";

class EngineAPI{
    inited = false

    PendingInitFunctions = []

    storeIdElementShapeMap = {}
    elementIdParentId = {}

    cppEngine = null

    activePlayer = null

    hasShape = false

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
        Logger.info("Engine inited!!!!!")

        this.PendingInitFunctions.forEach(func=>{
            func();
        })
    }

    CreateShape(shapeName){
        return this.cppEngine.CreateShape(shapeName)
    }

    DuplicateObject(obj){
        return this.cppEngine.DuplicateShape(obj)
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
        let _this = this
        let layer = this.GetCurrentStore().GetCurrentLayer();
        if(!layer.addShape){
            layer.addShape = (shape)=>{
                shape.update()
                layer.AddShapeInternal(shape.getRawShape())
                shape.isPermanent = true

                if(this.activePlayer){
                    this.activePlayer.getLayerShapes(layer).set(shape.getRawShape().ptr, shape)
                }

                _this.hasShape = true

                Logger.debug("Currently there're:" + layer.GetShapeCount() + " shapes in the layer.")
            }
        }
        return layer
    }

    RegisterElementShape(storeId, element){
        if(!this.storeIdElementShapeMap[storeId]){
            this.storeIdElementShapeMap[storeId] = new Set()
        }

        this.storeIdElementShapeMap[storeId].add(element)
    }

    GetElementShapeByStoreId(storeId){
        return this.storeIdElementShapeMap[storeId]
    }

    registerElementParent(childId, parentId){
        this.elementIdParentId[childId] = parentId
    }

    getElementParentByStoreId(childId){
        return this.elementIdParentId[childId]
    }

    registerEventListener(eventName, func){
        engineEventManager.registerEventListener(eventName, func)
    }

    dispatchEvent(eventName, ...params){
        engineEventManager.dispatchEvent(eventName, ...params)
    }

    DestroyShape(shape){
        this.cppEngine.DestroyShape(shape)
    }

    SetStoreFilePath(path){
        this.GetDefaultObjectStoreManager().SetStoreFilePath(path)
    }
}

let huahuoEngine = window.huahuoEngine
if(!window.huahuoEngine){
    huahuoEngine = new EngineAPI()
    window.huahuoEngine = huahuoEngine
}

export {huahuoEngine}