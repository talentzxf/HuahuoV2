import {HHTimeLine, TimelineEventNames, GlobalConfig} from "hhtimeline"
import {huahuoEngine} from "hhenginejs"
declare var Module:any;

class EditorPlayer{
    isPlaying: boolean = false
    timeline: HHTimeLine = null
    animationFrame = -1
    animationStartTime = -1

    lastAnimateTime = -1
    constructor() {
        this.timeline = document.querySelector("hh-timeline")
        this.timeline.addEventListener(TimelineEventNames.TRACKCELLCLICKED, this.updateAllShapes.bind(this))

        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            let keyFrameAddedHandler = new Module.ScriptEventHandlerImpl()
            keyFrameAddedHandler.handleEvent = _this.onKeyFrameAdded.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnKeyFrameAdded", keyFrameAddedHandler)
        })

        document.addEventListener('keydown', this.onKeyEvent.bind(this));
    }

    onKeyEvent(e){
        if(e.key == "Enter"){
            e.preventDefault()

            if(!this.isPlaying){
                this.startPlay()
            }else{
                this.stopPlay()
            }

            this.isPlaying = !this.isPlaying
        }
    }

    animationFrameStep(timeStamp){
        if(this.isPlaying){
            if(this.animationStartTime < 0){
                this.animationStartTime = timeStamp
            }
            let elapsedTime = timeStamp - this.animationStartTime

            if(this.lastAnimateTime < 0 || (elapsedTime - timeStamp ) > 1.0/GlobalConfig.fps){
                this.timeline.setTimeElapsed(elapsedTime/1000.0) // convert from miliseconds to seconds
                this.updateAllShapes()
                this.lastAnimateTime = timeStamp
            }

            requestAnimationFrame(this.animationFrameStep.bind(this));
        }
    }

    startPlay(){
        this.lastAnimateTime = -1
        this.animationStartTime = -1
        this.animationFrame = requestAnimationFrame(this.animationFrameStep.bind(this));
    }

    stopPlay(){
        if(this.animationFrame){
            cancelAnimationFrame(this.animationFrame)
        }else{
            console.log("Error, animation frame is invalid");
        }
    }

    onKeyFrameAdded(args){
        let keyframeAddedArgs = Module.wrapPointer(args, Module.KeyFrameAddedEventHandlerArgs)
        let layer = keyframeAddedArgs.GetLayer()
        let frameId = keyframeAddedArgs.GetFrameId()

        this.timeline.redrawCell(layer, frameId)
    }

    updateAllShapes(){
        let store = huahuoEngine.GetCurrentStore()
        let layerCount = store.GetLayerCount();
        for(let i = 0 ; i < layerCount; i++){
            let layer = store.GetLayer(i)
            huahuoEngine.updateLayerShapes(layer)
        }
    }
}

export {EditorPlayer}