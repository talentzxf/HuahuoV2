import {GlobalConfig} from "../GlobalConfig";
import {huahuoEngine} from "../EngineAPI";
import {LayerShapesManager} from "./LayerShapesManager";
import {IsValidWrappedObject, GraphEvent} from "hhcommoncomponents";
import {getNailManager} from '../IK/GetNailManager'
import {EventParam} from "hhcommoncomponents";
import {PropertyType, EventEmitter} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {getPhysicSystem} from "../PhysicsSystem/PhysicsSystem";

class Player extends EventEmitter {
    animationFrame = -1
    animationStartTime = -1

    lastAnimateTime = -1
    isPlaying: boolean = false

    _isInEditor: boolean = true
    layerShapesManager: LayerShapesManager;

    playStartTime: number = 0

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
            if(this.lastAnimateTime >= 0){
                elapsedTime = timeStamp - this.lastAnimateTime
            }

            if (this.lastAnimateTime < 0 || elapsedTime > 1000.0 / GlobalConfig.fps) {
                console.log("FPS:" + 1000.0/elapsedTime)
                let store = huahuoEngine.GetStoreById(this.storeId)
                let activeFrames = store.GetMaxFrameId() + 1;
                let activePlayTime = activeFrames / GlobalConfig.fps;
                let playTime = (timeStamp - this.animationStartTime + this.playStartTime * 1000.0) / 1000.0 % activePlayTime;
                let frameId = Math.floor(playTime * GlobalConfig.fps)
                this.setFrameId(frameId)
                this.lastAnimateTime = timeStamp

                getPhysicSystem().Step(elapsedTime/1000.0)
            }
            requestAnimationFrame(this.animationFrameStep.bind(this));
        }
    }

    @GraphEvent(true)
    setFrameId(@EventParam(PropertyType.NUMBER) playFrameId) {
        // Update time for all layers in the default store.
        let currentStore = huahuoEngine.GetStoreById(this.storeId)

        let layerCount = currentStore.GetLayerCount()
        for (let layerIdx = 0; layerIdx < layerCount; layerIdx++) {
            let layer = currentStore.GetLayer(layerIdx)
            layer.SetCurrentFrame(playFrameId)
        }

        this.updateAllShapes(true)

        getNailManager().update()

        this.currentlyPlayingFrameId = playFrameId
    }

    startPlay() {
        if (this.storeId == null) {
            this.storeId = huahuoEngine.GetCurrentStoreId()
        }

        let store = huahuoEngine.GetStoreById(this.storeId)
        this.playStartTime = store.GetCurrentLayer().GetCurrentFrame() / GlobalConfig.fps
        this.lastAnimateTime = -1
        this.animationFrame = requestAnimationFrame(this.animationFrameStep.bind(this));
        this.isPlaying = true
    }

    @GraphEvent(true)
    pausePlay() {
        if (this.animationFrame) {
            cancelAnimationFrame(this.animationFrame)
            this.animationStartTime = -1
        } else {
            console.log("Error, animation frame is invalid");
        }

        this.isPlaying = false
    }

    stopPlay() {
        this.pausePlay()
        this.setFrameId(0) // Reset to frame 0
        this.resetActions()
        getPhysicSystem().Reset()
    }

    resetActions() {
        this.layerShapesManager.forEachShapeInStore((shape: BaseShapeJS) => {
            shape.resetAction()
        })
    }
}

export {Player}