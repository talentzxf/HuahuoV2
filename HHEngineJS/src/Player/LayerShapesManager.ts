import {huahuoEngine} from "../EngineAPI";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {GetObjPtr, Logger} from "hhcommoncomponents"
import {LoadShapeFromCppShape} from "../Shapes/LoadShape";
import {ElementShapeJS} from "../Shapes/ElementShapeJS";

class LayerShapesManager {
    layerShapes = new Map();
    storeId: string;

    constructor(storeId) {
        this.storeId = storeId
    }

    removeShape(layer, obj) {
        if (!this.layerShapes.has(layer)) {
            Logger.error("Can't find layer!");
        } else {
            let shapesMap = this.layerShapes.get(layer)
            shapesMap.delete(GetObjPtr(obj))
        }
    }

    getLayerShapes(layer): Map<number, any> {

        if (!this.layerShapes.has(layer)) {
            this.layerShapes.set(layer, new Map<number, any>())
        }

        return this.layerShapes.get(layer)
    }

    updateLayerShapes(layer, force:boolean = false) {
        this.forEachShapeInLayer(layer, (shape) => {
            shape.update(force)
        })
    }

    hideLayerShapes(layer) {
        this.forEachShapeInLayer(layer, (shape) => {
            shape.hide()
        })
    }

    forEachLayerInStore(func: (layer, param?) => void, param?) {
        let store = huahuoEngine.GetStoreById(this.storeId)
        let layerCount = store.GetLayerCount();

        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)

            func(layer, param)
        }
    }

    forEachShapeInLayer(layer, func: (shape) => void) {
        let shapes = this.getLayerShapes(layer)
        for (let shape of shapes.values()) {
            func(shape)
        }
    }

    forEachShapeInStore(func: (shape) => void) {
        let store = huahuoEngine.GetStoreById(this.storeId)
        this.forEachLayerInStore((layer) => {
            this.forEachShapeInLayer(layer, func)
        })
    }

    getJSShapeFromRawShape(rawObj, recursive: boolean = false): BaseShapeJS {
        let store = huahuoEngine.GetStoreById(this.storeId)
        let layerCount = store.GetLayerCount();

        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)
            let shapes = this.getLayerShapes(layer)

            if (shapes.has(GetObjPtr(rawObj))) {
                return shapes.get(GetObjPtr(rawObj))
            }

            if (recursive) { // Shape might be an element, need to look for the shape recursively.
                for (let [shapePtr, shape] of shapes) {//
                    if (shape.layerShapesManager) { // The shape has layerShapes manager, look for the target.
                        let targetShape = shape.layerShapesManager.getJSShapeFromRawShape(rawObj, true)
                        if (targetShape != null)
                            return targetShape
                    }
                }
            }
        }

        return null
    }

    loadShapesFromStore(parent: ElementShapeJS): number {
        let layerShapeCount = 0

        if(parent != null)
            this.storeId = parent.storeId

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
                if (!shapes.has(GetObjPtr(baseShape))) { // TODO: This is duplicated with LoadShape.ts.
                    let jsShape = LoadShapeFromCppShape(baseShape)
                    shapes.set(GetObjPtr(baseShape), jsShape)
                    if (parent)
                        jsShape.setParent(parent)
                    shape = jsShape
                }
            }
        }

        // Remove all layers and shapes that are not belong to me. This might happen when a shape is removed from an element
        for (let layer of this.layerShapes.keys()) {
            if (layer.GetObjectStore().GetStoreId() != this.storeId) {
                this.layerShapes.delete(layer)
            } else {
                let shapes = this.layerShapes.get(layer)
                for (let shapePtr of shapes.keys()) {
                    let shape = shapes.get(shapePtr)
                    if (shape.belongStoreId.length !=0 && shape.belongStoreId != this.storeId) {
                        shape.removePaperObj()
                        shapes.delete(shapePtr)
                    }
                }
            }
        }

        return layerShapeCount
    }

    updateAllShapes(force: boolean = false) {
        this.forEachLayerInStore(this.updateLayerShapes.bind(this), force)
    }

    hideAllShapes() {
        this.forEachLayerInStore(this.hideLayerShapes.bind(this))
    }
}

export {LayerShapesManager}