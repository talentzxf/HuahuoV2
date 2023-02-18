import {Logger} from "hhcommoncomponents"
import {engineEventManager} from "./EngineEvents/EngineEventManager";
import {clzObjectFactory} from "./CppClassObjectFactory";
import * as ti from "taichi.js/dist/taichi"

class EngineAPI{
    inited = false

    PendingInitFunctions = []

    storeIdElementShapeMap = {}
    elementIdParentId = {}

    cppEngine = null

    activePlayer = null

    hasShape = false

    taichiInited = false

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

    async OnTaichiInit(){
        if(this.taichiInited){
            return
        }

        await ti.init()

        this.taichiInited = true

        Logger.info("Taichi Inited!!!!")

        if(this.inited && this.taichiInited){
            this.PendingInitFunctions.forEach(func=>{
                func();
            })
        }
    }

    OnInit(){
        if(this.inited)
            return;

        Module.HuaHuoEngine.prototype.InitEngine();
        this.cppEngine = Module.HuaHuoEngine.prototype.GetInstance();
        this.inited = true
        Logger.info("Engine inited!!!!!")

        if(this.inited && this.taichiInited){
            this.PendingInitFunctions.forEach(func=>{
                func();
            })
        }
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
        // FrameId starts from 0. So, maxFames = maxFrameId + 1
        return this.GetStoreById(storeId).GetMaxFrameId() + 1
    }

    GetStoreById(storeId){
        return this.GetDefaultObjectStoreManager().GetStoreById(storeId)
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

    getProjectWidth(){
        return this.GetDefaultObjectStoreManager().GetCanvasWidth();
    }

    getProjectHeight(){
        return this.GetDefaultObjectStoreManager().GetCanvasHeight();
    }

    setProjectWidthHeight(width, height){
        this.GetDefaultObjectStoreManager().SetCanvasWH(width, height)
    }

    getAllCompatibleComponents(targetObj){
        return clzObjectFactory.getAllCompatibleComponents(targetObj)
    }

    produceObject(componentName){
        let constructor = clzObjectFactory.GetClassConstructor(componentName)
        let retObj = new constructor()
        return retObj
    }

    get ti(){
        return ti
    }
}

let huahuoEngine = window.huahuoEngine
if(!window.huahuoEngine){
    huahuoEngine = new EngineAPI()
    window.huahuoEngine = huahuoEngine
}

export {huahuoEngine}