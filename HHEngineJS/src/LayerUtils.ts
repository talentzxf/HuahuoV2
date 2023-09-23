import {huahuoEngine} from "./EngineAPI";
import {Logger} from "hhcommoncomponents";
import {LayerGraphWrapper} from "./EventGraph/LayerGraphWrapper";

class LayerUtils {
    layerGraphWrapperObjMap

    initLayer(layer) {
        if (!layer.inited) {

            layer.addShape = (shape) => {
                shape.update()
                layer.AddShapeInternal(shape.getRawObject())
                shape.isPermanent = true
                shape.isDeleted = false

                if (huahuoEngine.getActivePlayer()) {
                    huahuoEngine.getActivePlayer().getLayerShapes(layer).set(shape.getRawObject().ptr, shape)
                }

                huahuoEngine.hasShape = true

                Logger.debug("Currently there're:" + layer.GetShapeCount() + " shapes in the layer.")
            }

            layer.inited = true
        }
    }

    getWrappedGraphObjectForLayer(layer, frameId, createIfNotExist = false) {
        let frameEventGraph = layer.GetFrameEventGraphParam(frameId, createIfNotExist)

        if (null == this.layerGraphWrapperObjMap) {
            this.layerGraphWrapperObjMap = new Map()
        }

        if (null == layer.GetFrameEventGraphParam(frameId)) {
            return null;
        }

        if (!this.layerGraphWrapperObjMap.has(layer)) {
            this.layerGraphWrapperObjMap.set(layer, new Map())
        }

        let frameIdGraphMap = this.layerGraphWrapperObjMap.get(layer)
        if (!frameIdGraphMap.has(frameId)) {
            frameIdGraphMap.set(frameId, new LayerGraphWrapper(frameEventGraph))
        }

        return frameIdGraphMap.get(frameId)
    }

    layerFrameIdCallbacks = new Map

    executePlayFrameCallbacks(layer, frameId) {
        if (this.layerFrameIdCallbacks.has(layer.ptr)) {
            if (this.layerFrameIdCallbacks.get(layer.ptr).has(frameId)) {
                let fnArray = this.layerFrameIdCallbacks.get(layer.ptr).get(frameId)
                for (let fn of fnArray) {
                    fn()
                }
            }
        }
    }

    addPlayFrameCallbacks(layer, frameId, callback) {
        if (!this.layerFrameIdCallbacks.has(layer.ptr)) {
            this.layerFrameIdCallbacks.set(layer.ptr, new Map)
        }

        if (!this.layerFrameIdCallbacks.get(layer.ptr).has(frameId)) {
            this.layerFrameIdCallbacks.get(layer.ptr).set(frameId, new Array)
        }

        this.layerFrameIdCallbacks.get(layer.ptr).get(frameId).push(callback)
    }
}

let layerUtils = window["layerUtils"]
if (!layerUtils) {
    layerUtils = new LayerUtils()
    window["layerUtils"] = layerUtils
}

export {layerUtils}