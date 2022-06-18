import {HHTimeLine, TimelineEventNames} from "hhtimeline"
import {huahuoEngine} from "hhenginejs"
declare var Module:any;

class EditorPlayer{
    timeline: HHTimeLine = null
    constructor() {
        this.timeline = document.querySelector("hh-timeline")
        this.timeline.addEventListener(TimelineEventNames.TRACKCELLCLICKED, this.updateAllShapes.bind(this))

        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            let keyFrameAddedHandler = new Module.ScriptEventHandlerImpl()
            keyFrameAddedHandler.handleEvent = _this.onKeyFrameAdded.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnKeyFrameAdded", keyFrameAddedHandler)

        })
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