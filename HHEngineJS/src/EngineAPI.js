import {dataURItoBlob, eventBus, getMimeTypeFromDataURI, IsValidWrappedObject, Logger} from "hhcommoncomponents"
import {clzObjectFactory} from "./CppClassObjectFactory";

// @ts-ignore
import * as ti from "taichi.js/dist/taichi"
import {BaseShapeEvents} from "./EventGraph/BaseShapeEvents";
import {BaseShapeJS} from "./Shapes/BaseShapeJS";
import {layerUtils} from "./LayerUtils";
// import * as ti from "taichi.js/dist/taichi.dev"


// Not sure why, but the instanceof failed in this case...
function isDerivedFrom(object, clz) {
    if (object instanceof clz)
        return true

    let targetClassName = clz.name
    let curObj = object
    while (curObj != null) {
        if (curObj.constructor.name == targetClassName)
            return true

        curObj = Object.getPrototypeOf(curObj)
    }

    return false
}

class EngineAPI {
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

    isPtrDeleted(shapePtr) {
        return this.deletedShapePtrs.has(shapePtr)
    }

    getEvent(eventEmitter) {
        if (!this.eventEmitterCache.has(eventEmitter)) {
            if (eventEmitter.rawObj instanceof Module.BaseShape) {
                this.eventEmitterCache.set(eventEmitter, new BaseShapeEvents(eventEmitter))
            } else {
                this.eventEmitterCache.set(eventEmitter, eventEmitter)
            }
        }
        return this.eventEmitterCache.get(eventEmitter)
    }

    getEventBus(eventEmitter) {
        if (isDerivedFrom(eventEmitter, BaseShapeJS))
            return this.getEvent(eventEmitter).getEventBus()

        // Return the global event bus
        return eventBus
    }

    constructor() {
        Logger.info("Creating Engine API!!!!")
    }

    setActivePlayer(activePlayer) {
        this.activePlayer = activePlayer
    }

    getActivePlayer() {
        return this.activePlayer
    }

    ExecuteAfterInited(func) {
        if (this.inited) {
            return func();
        }

        this.PendingInitFunctions.push(func);
    }

    GetInstance() {
        return this.cppEngine;
    }

    async OnTaichiInit() {
        if (this.taichiInited) {
            return
        }

        // TODO: Enable back when webgpu is supported on mobile phone.
        // await ti.init()

        this.taichiInited = true

        Logger.info("Taichi Inited!!!!")

        if (this.inited && this.taichiInited) {
            this.PendingInitFunctions.forEach(func => {
                func();
            })
        }
    }

    OnInit() {
        if (this.inited)
            return;

        Module.HuaHuoEngine.prototype.InitEngine();
        this.cppEngine = Module.HuaHuoEngine.prototype.GetInstance();
        this.inited = true
        Logger.info("Engine inited!!!!!")

        // Not sure why, but requestDevice of WebGPU always stuck??
        // if(this.inited && this.taichiInited){
        if (this.inited) {
            this.PendingInitFunctions.forEach(func => {
                func();
            })
        }
    }

    CreateShape(shapeName, createDefaultComponents = true) {
        return this.cppEngine.CreateShape(shapeName, createDefaultComponents)
    }

    DuplicateObject(obj) {
        return this.cppEngine.DuplicateShape(obj)
    }

    GetPersistentManager() {
        return Module.PersistentManager.prototype.GetPersistentManager()
    }

    GetDefaultObjectStoreManager() {
        return Module.ObjectStoreManager.prototype.GetDefaultObjectStoreManager()
    }

    GetCurrentStore() {
        return this.GetDefaultObjectStoreManager().GetCurrentStore()
    }

    GetCurrentStoreId() {
        return this.GetCurrentStore().GetStoreId()
    }

    getStoreMaxFrames(storeId) {
        // FrameId starts from 0. So, maxFrames = maxFrameId + 1
        return this.GetStoreById(storeId).GetMaxFrameId() + 1
    }

    shapeDecorator = null

    setShapeDecorator(shapeDecoratorFunc) {
        this.shapeDecorator = shapeDecoratorFunc
    }

    getShapeDecorator() {
        return this.shapeDecorator
    }

    GetStoreById(storeId) {
        return this.GetDefaultObjectStoreManager().GetStoreById(storeId)
    }

    GetCurrentLayer() {
        let layer = this.GetCurrentStore().GetCurrentLayer();
        layerUtils.initLayer(layer)
        return layer
    }

    RegisterElementShape(storeId, element) {
        if (!this.storeIdElementShapeMap[storeId]) {
            this.storeIdElementShapeMap[storeId] = new Set()
        }

        this.storeIdElementShapeMap[storeId].add(element)
    }

    GetElementShapeByStoreId(storeId) {
        return this.storeIdElementShapeMap[storeId]
    }

    registerElementParent(childId, parentId) {
        this.elementIdParentId[childId] = parentId
    }

    getElementParentByStoreId(childId) {
        return this.elementIdParentId[childId]
    }

    registerEventListener(namespace, eventName, func) {
        return eventBus.addEventHandler(namespace, eventName, func)
    }

    unregisterEventListener(namespace, eventName, handlerId) {
        eventBus.removeEventHandler(namespace, eventName, handlerId)
    }

    dispatchEvent(namespace, eventName, ...params) {
        eventBus.triggerEvent(namespace, eventName, ...params)
    }

    isValidShape(shape) {
        if (!IsValidWrappedObject(shape.rawObj))
            return false
        if (this.isPtrDeleted(shape.rawObj.ptr))
            return false

        return true
    }

    DestroyShape(rawShape) {
        if (!this.isPtrDeleted(rawShape.ptr)) {
            this.deletedShapePtrs.add(rawShape.ptr)
            this.cppEngine.DestroyShape(rawShape)
        }
    }

    getProjectWidth() {
        return this.GetDefaultObjectStoreManager().GetCanvasWidth();
    }

    getProjectHeight() {
        return this.GetDefaultObjectStoreManager().GetCanvasHeight();
    }

    setProjectWidthHeight(width, height) {
        this.GetDefaultObjectStoreManager().SetCanvasWH(width, height)
    }

    getAllCompatibleComponents(targetObj) {
        return clzObjectFactory.getAllCompatibleComponents(targetObj)
    }

    getAllRegisteredComponents() {
        return clzObjectFactory.getAllRegisteredComponents()
    }

    produceObject(componentName) {
        let constructor = clzObjectFactory.GetClassConstructor(componentName)
        let retObj = new constructor()
        return retObj
    }

    SetBinaryResource(resourceName, resourceData) {
        let mimeType = getMimeTypeFromDataURI(resourceData)

        let binaryData = dataURItoBlob(resourceData)
        this.cppEngine.SetFileData(resourceName, mimeType, binaryData, binaryData.length)
    }

    IsBinaryResourceExist(resourceMD5) {
        return this.cppEngine.IsBinaryResourceExist(resourceMD5)
    }

    LoadBinaryResource(fileName, mimeType, data, dataSize) {
        return this.cppEngine.LoadBinaryResource(fileName, mimeType, data, dataSize)
    }


    getWrappedGraphObjectForLayer(layer, frameId, createIfNotExist = false) {
        return layerUtils.getWrappedGraphObjectForLayer(layer, frameId, createIfNotExist)
    }

    get defaultFrameCount() {
        return 1000;
    }

    get ti() {
        return ti
    }
}

let huahuoEngine = window.huahuoEngine
if (!window.huahuoEngine) {
    huahuoEngine = new EngineAPI()
    window.huahuoEngine = huahuoEngine
}

export {huahuoEngine}