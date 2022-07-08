import {huahuoEngine} from "../EngineAPI";
import {BaseShapeJS, shapeFactory} from "../Shapes/BaseShapeJS";

class LayerShapesManager {
    layerShapes = new Map();

    getLayerShapes(layer) {

        if (!this.layerShapes.has(layer)) {
            this.layerShapes.set(layer, new Map())
        }

        return this.layerShapes.get(layer)
    }

    updateLayerShapes(layer) {
        this.forEachShapeInLayer(layer, (shape) => {
            shape.update()
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
                if (!shapes.has(baseShape)) {
                    let shapeConstructor = shapeFactory.GetShapeConstructor(baseShape.GetName())
                    let newBaseShape = shapeConstructor(baseShape)
                    newBaseShape.awakeFromLoad()
                    shapes.set(baseShape, newBaseShape)
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
}

export {LayerShapesManager}