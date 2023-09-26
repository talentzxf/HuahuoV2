import {GlobalConfig} from "../GlobalConfig";
import {huahuoEngine} from "../EngineAPI";
import {LayerShapesManager} from "./LayerShapesManager";
import {IsValidWrappedObject, GraphEvent} from "hhcommoncomponents";
import {getNailManager} from '../IK/GetNailManager'
import {EventParam} from "hhcommoncomponents";
import {PropertyType, EventEmitter} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {getPhysicSystem} from "../PhysicsSystem/PhysicsSystem";
import {LGraph} from "litegraph.js";
import {layerUtils} from "../LayerUtils";

class Player extends EventEmitter {
    animationFrame = -1
    animationStartTime = -1

    lastAnimateTime = -1

    // isPlaying == false && isPaused == false.  The animation stop at the beginning
    // isPlaying == false && isPaused == true.   The animation stopped at a certain frame.
    // isPlaying == true && isPaused == false.   The animation is playing.
    // isPlaying == true && isPaused == true.  Illegal state.
    isPlaying: boolean = false
    isPaused: boolean = false

    _isInEditor: boolean = true
    layerShapesManager: LayerShapesManager;

    public currentlyPlayingFrameId: number = 0

    set isInEditor(val: boolean) {
        this._isInEditor = val
    }

    get isInEditor(): boolean {
        return this._isInEditor
    }

    set storeId(val: string) {
        if (this.layerShapesManager == null) {
            this.layerShapesManager = new LayerShapesManager(val)
        }
        this.layerShapesManager.storeId = val
    }

    get storeId(): string {
        if (this.layerShapesManager == null)
            return null
        return this.layerShapesManager.storeId
    }

    constructor(storeId?) {
        super(true) // Player events are global

        if (storeId) {
            this.layerShapesManager = new LayerShapesManager(storeId)
        }
    }

    getLayerShapes(layer) {
        return this.layerShapesManager.getLayerShapes(layer)
    }

    loadShapesFromStore() {
        return this.layerShapesManager.loadShapesFromStore(null)
    }

    updateAllShapes(force: boolean = false) {
        this.layerShapesManager.updateAllShapes(force)
    }

    getJSShapeFromRawShape(rawObj, recursive: boolean = false) {
        if (!IsValidWrappedObject(rawObj))
            return null

        return this.layerShapesManager.getJSShapeFromRawShape(rawObj, recursive)
    }

    animationFrameStep(timeStamp) {
        if (this.isPlaying) {
            if (this.animationStartTime < 0) {
                this.animationStartTime = timeStamp
            }

            let elapsedTime = 0
            if (this.lastAnimateTime >= 0) {
                elapsedTime = timeStamp - this.lastAnimateTime
            }

            if (this.lastAnimateTime < 0 || elapsedTime > 1000.0 / GlobalConfig.fps) {
                if (elapsedTime > 0) {
                    getPhysicSystem().Step(elapsedTime / 1000.0)
                }

                let store = huahuoEngine.GetStoreById(this.storeId)
                let activeFrames = store.GetMaxFrameId() + 1;

                let deltaFrames = elapsedTime / GlobalConfig.fps
                let frameId = Math.floor(this.currentlyPlayingFrameId + deltaFrames + activeFrames) % activeFrames
                console.log("Debug Jump Frame: deltaFrames:" + deltaFrames + ",frameId" + frameId)
                this.setFrameId(frameId, false)
                this.lastAnimateTime = timeStamp
            }
            requestAnimationFrame(this.animationFrameStep.bind(this));
        }
    }

    @GraphEvent(true)
    setFrameId(@EventParam(PropertyType.NUMBER) playFrameId, forceSyncLayers: boolean = true) {
        let lastPlayingFrameId = this.currentlyPlayingFrameId
        this.currentlyPlayingFrameId = playFrameId

        // Update time for all layers in the default store.
        let currentStore = huahuoEngine.GetStoreById(this.storeId)

        let deltaFrameCount = playFrameId - lastPlayingFrameId

        let layerCount = currentStore.GetLayerCount()
        for (let layerIdx = 0; layerIdx < layerCount; layerIdx++) {
            let layer = currentStore.GetLayer(layerIdx)
            let currentLayerFrameId = layer.GetCurrentFrame()

            let nextFrameId = playFrameId
            if (!forceSyncLayers && deltaFrameCount > 0) {
                let currentLayerFrameId = layer.GetCurrentFrame();
                nextFrameId = currentLayerFrameId + deltaFrameCount
            }

            if (nextFrameId != layer.GetCurrentFrame()) { // Only setFrameId when current frameId is not equal to the nextFrameId, to avoid infinite recursion.
                console.log("Debug Jump Frame: nextFrameId:" + nextFrameId)

                if (!forceSyncLayers && layer.IsStopFrame(currentLayerFrameId)) {
                    continue
                }

                layer.SetCurrentFrame(nextFrameId)

                for (let frameId = currentLayerFrameId + 1; frameId <= nextFrameId; frameId++) {
                    layerUtils.executePlayFrameCallbacks(layer, nextFrameId)
                }
            }
        }

        this.updateAllShapes(true)

        getNailManager().update()
    }

    startPlay() {
        if (this.storeId == null) {
            this.storeId = huahuoEngine.GetCurrentStoreId()
        }

        let store = huahuoEngine.GetStoreById(this.storeId)
        this.currentlyPlayingFrameId = store.GetCurrentLayer().GetCurrentFrame()
        this.lastAnimateTime = -1
        this.animationFrame = requestAnimationFrame(this.animationFrameStep.bind(this));
        this.isPlaying = true
        this.isPaused = false
    }

    @GraphEvent(true)
    pausePlay() {
        if (this.animationFrame) {
            cancelAnimationFrame(this.animationFrame)
            this.animationStartTime = -1
        } else {
            console.log("Error, animation frame is invalid");
        }

        this.isPaused = true
        this.isPlaying = false
    }

    stopPlay() {
        this.pausePlay()
        this.setFrameId(0) // Reset to frame 0
        this.resetActions()
        getPhysicSystem().Reset()

        this.isPaused = false
    }

    resetActions() {
        this.layerShapesManager.forEachShapeInStore((shape: BaseShapeJS) => {
            shape.resetAction()
        })
    }
}

export {Player}