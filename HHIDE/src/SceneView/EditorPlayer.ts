import {HHTimeLine, TimelineEventNames} from "hhtimeline"
import {huahuoEngine, GlobalConfig} from "hhenginejs"
import {Player} from "hhenginejs/src/Player/Player"
declare var Module:any;

class EditorPlayer extends Player{

    timeline: HHTimeLine = null

    constructor() {
        super()

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
        if(e.key == "Enter" && e.ctrlKey){ // Ctrl+Enter
            e.preventDefault()

            if(!this.isPlaying){
                this.startPlay()
            }else{
                this.stopPlay()
            }
        }
    }

    onPlayFrame(elapsedTime){
        super.onPlayFrame(elapsedTime)
        let activeFrames = huahuoEngine.GetCurrentStore().GetMaxFrameId() + 1
        let activePlayTime = activeFrames / GlobalConfig.fps
        let playTime = elapsedTime/1000.0 % activePlayTime
        this.timeline.setTimeElapsed(playTime) // convert from miliseconds to seconds
    }


    onKeyFrameAdded(args){
        let keyframeAddedArgs = Module.wrapPointer(args, Module.KeyFrameAddedEventHandlerArgs)
        let layer = keyframeAddedArgs.GetLayer()
        let frameId = keyframeAddedArgs.GetFrameId()

        this.timeline.redrawCell(layer, frameId)
    }

}

export {EditorPlayer}