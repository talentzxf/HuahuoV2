import {GlobalConfig} from "../GlobalConfig";
import {huahuoEngine} from "../EngineAPI";
import {LayerShapesManager} from "./LayerShapesManager";

class Player{
    animationFrame = -1
    animationStartTime = -1

    lastAnimateTime = -1
    isPlaying: boolean = false
    layerShapesManager: LayerShapesManager = new LayerShapesManager()

    playStartTime: number = 0

    public currentlyPlayingFrameId: number = -1

    constructor() {
    }

    getLayerShapes(layer){
        return this.layerShapesManager.getLayerShapes(layer)
    }

    loadShapesFromStore(){
        return this.layerShapesManager.loadShapesFromStore(null)
    }

    updateAllShapes(){
        this.layerShapesManager.updateAllShapes()
    }

    getJSShapeFromRawShape(rawObj){
        return this.layerShapesManager.getJSShapeFromRawShape(rawObj)
    }

    animationFrameStep(timeStamp){
        if(this.isPlaying){
            if(this.animationStartTime < 0){
                this.animationStartTime = timeStamp
            }
            let elapsedTime = timeStamp - this.lastAnimateTime

            console.log("Elapsed time:" + elapsedTime)
            console.log("Expected time:" + 1000.0/GlobalConfig.fps)

            if(this.lastAnimateTime < 0 || elapsedTime > 1000.0/GlobalConfig.fps){
                let activeFrames = huahuoEngine.GetCurrentStore().GetMaxFrameId() + 1;
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

    setFrameId(playFrameId){
        // Update time for all layers in the default store.
        let currentStore = huahuoEngine.GetCurrentStore()

        let layerCount = currentStore.GetLayerCount()
        for(let layerIdx = 0; layerIdx < layerCount; layerIdx++){
            let layer = currentStore.GetLayer(layerIdx)
            layer.SetCurrentFrame(playFrameId)
        }

        this.updateAllShapes()

        this.currentlyPlayingFrameId = playFrameId
    }

    startPlay(){
        this.playStartTime = huahuoEngine.GetCurrentLayer().GetCurrentFrame() / GlobalConfig.fps
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