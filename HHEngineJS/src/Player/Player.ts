import {GlobalConfig} from "../GlobalConfig";
import {huahuoEngine} from "../EngineAPI";

class Player{
    animationFrame = -1
    animationStartTime = -1

    lastAnimateTime = -1
    isPlaying: boolean = false

    constructor() {
        huahuoEngine.setActivePlayer(this)
    }

    updateLayerShapes(layer){
        let shapes = this.getLayerShapes(layer)
        for(let shape of shapes){
            shape.update()
        }
    }

    getLayerShapes(layer){

        if(!this.layerShapes.has(layer)){
            this.layerShapes.set(layer, [])
        }

        return this.layerShapes.get(layer)
    }

    updateAllShapes(){
        let store = huahuoEngine.GetCurrentStore()
        let layerCount = store.GetLayerCount();
        for(let i = 0 ; i < layerCount; i++){
            let layer = store.GetLayer(i)
            this.updateLayerShapes(layer)
        }
    }

    animationFrameStep(timeStamp){
        if(this.isPlaying){
            if(this.animationStartTime < 0){
                this.animationStartTime = timeStamp
            }
            let elapsedTime = timeStamp - this.animationStartTime

            if(this.lastAnimateTime < 0 || (elapsedTime - this.lastAnimateTime ) > 1.0/GlobalConfig.fps){

                this.onPlayFrame(elapsedTime)
            }

            requestAnimationFrame(this.animationFrameStep.bind(this));
        }
    }

    onPlayFrame(elapsedTime){
        this.updateAllShapes()
        this.lastAnimateTime = elapsedTime
    }

    startPlay(){
        this.lastAnimateTime = -1
        this.animationStartTime = -1
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

    layerShapes = new Map();
}

export {Player}