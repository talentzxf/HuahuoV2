import {Logger} from "hhcommoncomponents"
import {clzObjectFactory} from "./CppClassObjectFactory";
import {getMimeTypeFromDataURI} from "hhcommoncomponents";
import {dataURItoBlob, eventBus} from "hhcommoncomponents";

// @ts-ignore
import * as ti from "taichi.js/dist/taichi"
import {BaseShapeEvents} from "./EventGraph/BaseShapeEvents";
import {IsValidWrappedObject} from "hhcommoncomponents";
// import * as ti from "taichi.js/dist/taichi.dev"

class EngineAPI{
    inited = false

    PendingInitFunctions = []

    storeIdElementShapeMap = {}
    elementIdParentId = {}

    cppEngine = null

    activePlayer = null

    hasShape = false

    taichiInited = false

    eventEmitterCache = new Map

    deletedShapePtrs = new Set

    isPtrDeleted(shapePtr){
        return this.deletedShapePtrs.has(shapePtr)
    }

    getEvent(shape){
        if(!this.eventEmitterCache.has(shape))
            this.eventEmitterCache.set(shape, new BaseShapeEvents(shape))
        return this.eventEmitterCache.get(shape)
    }

    getEventBus(shape){
        return this.getEvent(shape).getEventBus()
    }

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

        // Not sure why, but requestDevice of WebGPU always stuck??
        // if(this.inited && this.taichiInited){
        if(this.inited){
            this.PendingInitFunctions.forEach(func=>{
                func();
            })
        }
    }

    CreateShape(shapeName, createDefaultComponents = true){
        return this.cppEngine.CreateShape(shapeName, createDefaultComponents)
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
        // FrameId starts from 0. So, maxFrames = maxFrameId + 1
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

    registerEventListener(namespace, eventName, func){
        return eventBus.addEventHandler(namespace, eventName, func)
    }

    unregisterEventListener(namespace, eventName, handlerId){
        eventBus.removeEventHandler(namespace, eventName, handlerId)
    }

    dispatchEvent(namespace, eventName, ...params){
        eventBus.triggerEvent(namespace, eventName, ...params)
    }

    isValidShape(shape){
        if(!IsValidWrappedObject(shape.rawObj))
            return false
        if(this.isPtrDeleted(shape.rawObj.ptr))
            return false

        return true
    }

    DestroyShape(rawShape){
        if(!this.isPtrDeleted(rawShape.ptr)){
            this.deletedShapePtrs.add(rawShape.ptr)
            this.cppEngine.DestroyShape(rawShape)
        }
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
    
    SetBinaryResource(resourceName, resourceData){
        let mimeType = getMimeTypeFromDataURI(resourceData)

        let binaryData = dataURItoBlob(resourceData)
        this.cppEngine.SetFileData(resourceName, mimeType, binaryData, binaryData.length)
    }

    IsBinaryResourceExist(resourceMD5){
        return this.cppEngine.IsBinaryResourceExist(resourceMD5)
    }

    LoadBinaryResource(fileName, mimeType, data, dataSize){
        return this.cppEngine.LoadBinaryResource(fileName, mimeType, data, dataSize)
    }

    get defaultFrameCount(){
        return 1000;
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