import {GlobalConfig} from "../GlobalConfig";
import {huahuoEngine} from "../EngineAPI";
import {LayerShapesManager} from "./LayerShapesManager";

class Player{
    animationFrame = -1
    animationStartTime = -1

    lastAnimateTime = -1
    isPlaying: boolean = false
    layerShapesManager: LayerShapesManager = new LayerShapesManager()

    constructor() {
        huahuoEngine.setActivePlayer(this)
    }

    getLayerShapes(layer){
        return this.layerShapesManager.getLayerShapes(layer)
    }

    updateAllShapes(){
        this.layerShapesManager.updateAllShapes()
    }

    animationFrameStep(timeStamp){
        if(this.isPlaying){
            if(this.animationStartTime < 0){
                this.animationStartTime = timeStamp
            }
            let elapsedTime = timeStamp - this.animationStartTime

            if(this.lastAnimateTime < 0 || (elapsedTime - this.lastAnimateTime ) > 1.0/GlobalConfig.fps){
                let activeFrames = huahuoEngine.GetCurrentStore().GetMaxFrameId() + 1;
                let activePlayTime = activeFrames / GlobalConfig.fps;
                let playTime = elapsedTime / 1000.0 % activePlayTime;
                let frameId = Math.floor(playTime * GlobalConfig.fps)
                this.onPlayFrame(frameId)

                this.lastAnimateTime = elapsedTime
            }

            requestAnimationFrame(this.animationFrameStep.bind(this));
        }
    }

    onPlayFrame(playFrameId){
        // Update time for all layers in the default store.
        let currentStore = huahuoEngine.GetCurrentStore()

        let layerCount = currentStore.GetLayerCount()
        for(let layerIdx = 0; layerIdx < layerCount; layerIdx++){
            let layer = currentStore.GetLayer(layerIdx)
            layer.SetCurrentFrame(playFrameId)
        }

        this.updateAllShapes()
    }

    startPlay(){
        this.lastAnimateTime = -1
        this.animationStartTime = huahuoEngine.GetCurrentLayer().GetCurrentFrame() / GlobalConfig.fps;
        this.animationFrame = requestAnimationFrame(this.animationFrameStep.bind(this));

        this.isPlaying = true
    }

    stopPlay(){
        if(this.animationFrame){
            cancelAnimationFrame(this.animationFrame)
        }else{
            console.log("Error, animation frame is invalid");
        }

        this.isPlaying = false
    }
}

export {Player}