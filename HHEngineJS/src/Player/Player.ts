import {GlobalConfig} from "../GlobalConfig";
import {huahuoEngine} from "../EngineAPI";
import {LayerShapesManager} from "./LayerShapesManager";
import {IsValidWrappedObject, TriggerEvent} from "hhcommoncomponents";
import {getNailManager} from '../IK/GetNailManager'
import {EventOut, EventParameterType} from "hhcommoncomponents";

class Player{
    animationFrame = -1
    animationStartTime = -1

    lastAnimateTime = -1
    isPlaying: boolean = false
    layerShapesManager: LayerShapesManager;

    playStartTime: number = 0

    public currentlyPlayingFrameId: number = 0

    set storeId(val: number){
        this.layerShapesManager.storeId = val
    }

    get storeId():number{
        return this.layerShapesManager.storeId
    }

    constructor(storeId) {
        this.layerShapesManager = new LayerShapesManager(storeId)
    }

    getLayerShapes(layer){
        return this.layerShapesManager.getLayerShapes(layer)
    }

    loadShapesFromStore(){
        return this.layerShapesManager.loadShapesFromStore(null)
    }

    updateAllShapes(force: boolean = false){
        this.layerShapesManager.updateAllShapes(force)
    }

    getJSShapeFromRawShape(rawObj, recursive: boolean = false){
        if(!IsValidWrappedObject(rawObj))
            return null

        return this.layerShapesManager.getJSShapeFromRawShape(rawObj, recursive)
    }

    animationFrameStep(timeStamp){
        if(this.isPlaying){
            if(this.animationStartTime < 0){
                this.animationStartTime = timeStamp
            }
            let elapsedTime = timeStamp - this.lastAnimateTime

            console.log("Elapsed time:" + elapsedTime)

            if(this.lastAnimateTime < 0 || elapsedTime > 1000.0/GlobalConfig.fps){
                let store = huahuoEngine.GetStoreById(this.storeId)
                let activeFrames = store.GetMaxFrameId() + 1;
                let activePlayTime = activeFrames / GlobalConfig.fps;
                let playTime = (timeStamp - this.animationStartTime + this.playStartTime * 1000.0) / 1000.0 % activePlayTime;
                let frameId = Math.floor(playTime * GlobalConfig.fps)
                this.setFrameId(frameId)
                console.log("Rendering")
                this.lastAnimateTime = timeStamp
            }else{
                console.log("Skipped Rendering")
            }
            requestAnimationFrame(this.animationFrameStep.bind(this));
        }
    }

    @TriggerEvent()
    setFrameId(@EventOut(EventParameterType.Integer) playFrameId){
        // Update time for all layers in the default store.
        let currentStore = huahuoEngine.GetStoreById(this.storeId)

        let layerCount = currentStore.GetLayerCount()
        for(let layerIdx = 0; layerIdx < layerCount; layerIdx++){
            let layer = currentStore.GetLayer(layerIdx)
            layer.SetCurrentFrame(playFrameId)
        }

        this.updateAllShapes()

        getNailManager().update()

        this.currentlyPlayingFrameId = playFrameId
    }

    startPlay(){
        let store = huahuoEngine.GetStoreById(this.storeId)
        this.playStartTime = store.GetCurrentLayer().GetCurrentFrame() / GlobalConfig.fps
        this.lastAnimateTime = -1
        this.animationFrame = requestAnimationFrame(this.animationFrameStep.bind(this));
        this.isPlaying = true
    }

    stopPlay(){
        if(this.animationFrame){
            cancelAnimationFrame(this.animationFrame)
            this.animationStartTime = -1
        }else{
            console.log("Error, animation frame is invalid");
        }

        this.isPlaying = false
    }
}

export {Player}