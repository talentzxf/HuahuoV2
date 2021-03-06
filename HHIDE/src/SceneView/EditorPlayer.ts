import {HHTimeLine, TimelineEventNames} from "hhtimeline"
import {huahuoEngine, GlobalConfig} from "hhenginejs"
import {Player} from "hhenginejs/src/Player/Player"
import {SceneView} from "./SceneView";

declare var Module:any;

class EditorPlayer extends Player{

    timeline: HHTimeLine = null
    sceneView: SceneView = null

    constructor(sceneView) {
        super()

        this.sceneView = sceneView

        this.timeline = document.querySelector("hh-timeline")
        this.timeline.addEventListener(TimelineEventNames.TRACKCELLCLICKED, this.onSetFrameTime.bind(this))

        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            let keyFrameAddedHandler = new Module.ScriptEventHandlerImpl()
            keyFrameAddedHandler.handleEvent = _this.onKeyFrameAdded.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnKeyFrameAdded", keyFrameAddedHandler)
        })

        document.addEventListener('keydown', this.onKeyEvent.bind(this));
    }

    onSetFrameTime(e){
        let elapsedTime = e.detail.elapsedTime
        let defaultStore = huahuoEngine.GetCurrentStore()
        let layerCount = defaultStore.GetLayerCount()
        for(let layerIdx = 0 ; layerIdx < layerCount; layerIdx++){
            let layer = defaultStore.GetLayer(layerIdx)
            layer.SetCurrentFrame(Math.floor(elapsedTime * GlobalConfig.fps))
        }

        this.updateAllShapes()
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

    onPlayFrame(playFrameId){
        super.onPlayFrame(playFrameId)
        this.timeline.setTimeElapsed(playFrameId / GlobalConfig.fps)
    }

    onKeyFrameAdded(args){
        let keyframeAddedArgs = Module.wrapPointer(args, Module.KeyFrameAddedEventHandlerArgs)
        let layer = keyframeAddedArgs.GetLayer()
        let frameId = keyframeAddedArgs.GetFrameId()

        this.timeline.redrawCell(layer, frameId)
    }
}

export {EditorPlayer}