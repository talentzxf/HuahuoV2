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

    GetCurrentLayer(){
        let layer = this.GetCurrentStore().GetCurrentLayer();
        if(!layer.addShape){
            layer.addShape = (shape)=>{
                shape.update()
                layer.AddShapeInternal(shape.getRawShape())

                this.getLayerShapes(layer).push(shape)
                Logger.debug("Currently there're:" + layer.GetShapeCount() + " shapes in the layer.")
            }
        }
        return layer
    }

    updateLayerShapes(layer){
        let shapes = this.getLayerShapes(layer)
        for(let shape of shapes){
            shape.update()
        }
    }

    getLayerShapes(layer){
        if(!this.layerShapes.has(layer)){
            this.layerShapes.set(layer, [])
        }

        return this.layerShapes.get(layer)
    }

    layerShapes = new Map();
}

let huahuoEngine = window.huahuoEngine
if(!window.huahuoEngine){
    huahuoEngine = new EngineAPI()
    window.huahuoEngine = huahuoEngine
}

export {huahuoEngine}