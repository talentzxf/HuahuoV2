import {huahuoEngine} from "../EngineAPI";
import {BaseShapeJS, shapeFactory} from "../Shapes/BaseShapeJS";
import {Logger} from "hhcommoncomponents"

class LayerShapesManager {
    layerShapes = new Map();

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

    getJSShapeFromRawShape(rawObj):BaseShapeJS{
        let store = huahuoEngine.GetCurrentStore()

        let layerCount = store.GetLayerCount();

        let retShape:BaseShapeJS = null;
        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)
            let shapes = this.getLayerShapes(layer)

            if(shapes.has(rawObj.ptr)){
                return shapes.get(rawObj.ptr)
            }
        }

        return retShape
    }

    loadShapesFromStore(parent: BaseShapeJS): number {
        let layerShapeCount = 0

        let store = huahuoEngine.GetCurrentStore()

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
                if (!shapes.has(baseShape.ptr)) {
                    let shapeConstructor = shapeFactory.GetShapeConstructor(baseShape.GetTypeName())
                    let newBaseShape = shapeConstructor(baseShape)
                    newBaseShape.awakeFromLoad()
                    shapes.set(baseShape.ptr, newBaseShape)
                    if (parent)
                        newBaseShape.setParent(parent)
                    shape = newBaseShape
                }
            }
        }
        return layerShapeCount
    }

    updateAllShapes() {
        let store = huahuoEngine.GetCurrentStore()
        this.forEachLayerInStore(store, this.updateLayerShapes.bind(this))
    }

    hideAllShapes(){
        let store = huahuoEngine.GetCurrentStore()
        this.forEachLayerInStore(store, this.hideLayerShapes.bind(this))
    }
}

export {LayerShapesManager}