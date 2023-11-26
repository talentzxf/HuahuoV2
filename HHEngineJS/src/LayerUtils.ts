import {huahuoEngine} from "./EngineAPI";
import {GetObjPtr, IsValidWrappedObject, Logger} from "hhcommoncomponents";
import {LayerGraphWrapper} from "./EventGraph/LayerGraphWrapper";

class LayerUtils {
    layerGraphWrapperObjMap

    initLayer(layer) {
        if (!layer.inited) {

            layer.addShape = (shape) => {
                shape.update()

                console.log(layer.GetTimeLineCellManager().GetSpanHead(20))
                layer.AddShapeInternal(shape.getRawObject())
                console.log(layer.GetTimeLineCellManager().GetSpanHead(20))

                shape.isPermanent = true
                shape.isDeleted = false

                if (huahuoEngine.getActivePlayer()) {
                    huahuoEngine.getActivePlayer().getLayerShapes(layer).set(GetObjPtr(shape), shape)
                }

                huahuoEngine.hasShape = true

                Logger.debug("Currently there're:" + layer.GetShapeCount() + " shapes in the layer.")
            }

            layer.inited = true
        }
    }

    getWrappedGraphObjectForLayer(layer, frameId, createIfNotExist = false) {
        let frameEventGraph = layer.GetFrameEventGraphParam(frameId, createIfNotExist)

        if (!IsValidWrappedObject(frameEventGraph))
            return null

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

    reset() {
        if (this.layerGraphWrapperObjMap) {
            this.layerGraphWrapperObjMap.forEach((layerWrapperMap) => {
                if (layerWrapperMap) {
                    layerWrapperMap.forEach((wrapper: LayerGraphWrapper) => {
                        wrapper.reset()
                    })
                }
            })
        }
    }

    layerFrameIdCallbacks = new Map

    executePlayFrameCallbacks(layer, frameId) {
        if (this.layerFrameIdCallbacks.has(GetObjPtr(layer))) {
            if (this.layerFrameIdCallbacks.get(GetObjPtr(layer)).has(frameId)) {
                let fnArray = this.layerFrameIdCallbacks.get(GetObjPtr(layer)).get(frameId)
                for (let fn of fnArray) {
                    // console.log("Debug Jump frame: Executing graph of frame:" + frameId)
                    fn()
                }
            }
        }
    }

    addPlayFrameCallbacks(layer, frameId, callback) {
        if (!this.layerFrameIdCallbacks.has(GetObjPtr(layer))) {
            this.layerFrameIdCallbacks.set(GetObjPtr(layer), new Map)
        }

        if (!this.layerFrameIdCallbacks.get(GetObjPtr(layer)).has(frameId)) {
            this.layerFrameIdCallbacks.get(GetObjPtr(layer)).set(frameId, new Array)
        }

        this.layerFrameIdCallbacks.get(GetObjPtr(layer)).get(frameId).push(callback)
    }

    uniformFrameId(frameId, maxFrameId) {
        return Math.floor(((frameId + maxFrameId) % maxFrameId + maxFrameId) % maxFrameId)
    }

    // Return true -- The lay is set to the target frameId.
    // Return false -- The lay is not set to the target frameId because of various reasons.
    advanceLayerFrameId(layer, globalTargetFrameId, currentLayerFrame, forceSync: boolean = true, globalPrevFrameId = -1, isForward = true) {
        if (!IsValidWrappedObject(layer))
            return false

        // get the wrapped object so it will execute the addPlayFrameCallbacks event.
        huahuoEngine.getWrappedGraphObjectForLayer(layer, globalTargetFrameId, false)

        // if (globalTargetFrameId == globalPrevFrameId)
        //     return false

        let totalFrames = layer.GetObjectStore().GetMaxFrameId() + 1

        if (totalFrames == 0) { // If currently no frame, enter force sync mode.
            forceSync = true
        }

        let nextFrameId = globalTargetFrameId
        if (!forceSync) { // If not force sync, will update the frame count by delta.
            if (layer.IsStopFrame(currentLayerFrame)) {
                layer.SetCurrentFrame(currentLayerFrame) // Stop there.
                return false
            }

            let deltaCount = -1
            if (isForward) {
                deltaCount = this.uniformFrameId(globalTargetFrameId - globalPrevFrameId, totalFrames)
            } else {
                deltaCount = this.uniformFrameId(globalPrevFrameId - globalTargetFrameId, totalFrames)
            }

            let jumped = false
            let forwardSignal = isForward ? 1 : -1

            for (let deltaFrameId = 0; deltaFrameId != deltaCount + 1; deltaFrameId++) {
                let candidateFrameId = this.uniformFrameId(currentLayerFrame + deltaFrameId * forwardSignal, totalFrames)
                let candidateNextFrameId = layer.GetNextFrameId(candidateFrameId)
                if (candidateNextFrameId >= 0) {
                    nextFrameId = candidateNextFrameId
                    jumped = true
                    break
                }
            }

            if (!jumped) {
                nextFrameId = this.uniformFrameId(currentLayerFrame + deltaCount * forwardSignal, totalFrames)
            }
        }

        if (layer.GetCurrentFrame() != nextFrameId) {
            layer.SetCurrentFrame(nextFrameId)

            if (!forceSync) { // Only execute callback when it's not a forceSync. Because executing callback might impact currentFrame.
                let frameId = currentLayerFrame
                do {
                    frameId = this.uniformFrameId(isForward ? frameId + 1 : frameId - 1, totalFrames)
                    this.executePlayFrameCallbacks(layer, frameId)
                } while (frameId != nextFrameId)
            }
        }

        return true
    }
}

let layerUtils = window["layerUtils"]
if (!layerUtils) {
    layerUtils = new LayerUtils()
    window["layerUtils"] = layerUtils
}

export {layerUtils}