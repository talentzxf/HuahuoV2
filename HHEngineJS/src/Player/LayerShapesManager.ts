import {huahuoEngine} from "../EngineAPI";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {Logger} from "hhcommoncomponents"
import {LoadShapeFromCppShape} from "../Shapes/LoadShape";

class LayerShapesManager {
    layerShapes = new Map();
    storeId:number = -1;

    constructor(storeId) {
        this.storeId = storeId
    }

    removeShape(layer, obj){
        if(!this.layerShapes.has(layer)){
            Logger.error("Can't find layer!");
        }else{
            let shapesMap = this.layerShapes.get(layer)
            shapesMap.delete(obj.ptr)
        }
    }

    getLayerShapes(layer):Map<number, any> {

        if (!this.layerShapes.has(layer)) {
            this.layerShapes.set(layer, new Map<number, any>())
        }

        return this.layerShapes.get(layer)
    }

    updateLayerShapes(layer) {
        this.forEachShapeInLayer(layer, (shape) => {
            shape.update()
        })
    }

    hideLayerShapes(layer){
        this.forEachShapeInLayer(layer, (shape) => {
            shape.hide()
        })
    }

    forEachLayerInStore(store, func: (layer) => void) {
        let layerCount = store.GetLayerCount();

        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)

            func(layer)
        }
    }

    forEachShapeInLayer(layer, func: (shape) => void) {
        let shapes = this.getLayerShapes(layer)
        for (let shape of shapes.values()) {
            func(shape)
        }
    }

    forEachShapeInStore(store, func: (shape) => void) {
        this.forEachLayerInStore(store, (layer) => {
            this.forEachShapeInLayer(layer, func)
        })
    }

    getJSShapeFromRawShape(rawObj, recursive: boolean = false):BaseShapeJS{
        let store = huahuoEngine.GetStoreById(this.storeId)

        let layerCount = store.GetLayerCount();

        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)
            let shapes = this.getLayerShapes(layer)

            if(shapes.has(rawObj.ptr)){
                return shapes.get(rawObj.ptr)
            }

            if(recursive){ // Shape might be an element, need to look for the shape recursively.
                for(let [shapePtr, shape] of shapes){//
                    if(shape.layerShapesManager){ // The shape has layerShapes manager, look for the target.
                        let targetShape = shape.layerShapesManager.getJSShapeFromRawShape(rawObj, true)
                        if(targetShape != null)
                            return targetShape
                    }
                }
            }
        }

        return null
    }

    loadShapesFromStore(parent: BaseShapeJS): number {
        let layerShapeCount = 0

        let store = huahuoEngine.GetStoreById(this.storeId)

        let layerCount = store.GetLayerCount();

        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)
            let shapes = this.getLayerShapes(layer)

            // Try to create layer shapes
            let shapeCount = layer.GetShapeCount()
            for (let shapeId = 0; shapeId < shapeCount; shapeId++) {
                layerShapeCount++
                let baseShape = layer.GetShapeAtIndex(shapeId)
                let shape = null
                if (!shapes.has(baseShape.ptr)) { // TODO: This is duplicated with LoadShape.ts.
                    let jsShape = LoadShapeFromCppShape(baseShape)
                    shapes.set(baseShape.ptr, jsShape)
                    if (parent)
                        jsShape.setParent(parent)
                    shape = jsShape
                }
            }

            for(let [shapePtr, shape] of this.getLayerShapes(layer)){
                shape.update()
            }
        }

        // Remove all layers and shapes that are not belong to me. This might happen when a shape is removed from an element
        for(let layer of this.layerShapes.keys()){
            if(layer.GetObjectStore().GetStoreId() != this.storeId){
                this.layerShapes.delete(layer)
            } else{
                let shapes = this.layerShapes.get(layer)
                for(let shapePtr of shapes.keys()){
                    let shape = shapes.get(shapePtr)
                    if(shape.belongStoreId != this.storeId){
                        shape.removePaperObj()
                        shapes.delete(shapePtr)
                    }
                }
            }
        }

        return layerShapeCount
    }

    updateAllShapes() {
        let store = huahuoEngine.GetStoreById(this.storeId)
        this.forEachLayerInStore(store, this.updateLayerShapes.bind(this))
    }

    hideAllShapes(){
        let store = huahuoEngine.GetStoreById(this.storeId)
        this.forEachLayerInStore(store, this.hideLayerShapes.bind(this))
    }
}

export {LayerShapesManager}