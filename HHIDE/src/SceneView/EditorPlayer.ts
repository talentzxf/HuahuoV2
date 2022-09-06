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

        this.timeline = sceneView.timeline
        this.timeline.addEventListener(TimelineEventNames.TRACKCELLCLICKED, this.onSetFrameTime.bind(this))

        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            let keyFrameChangedHandler = new Module.ScriptEventHandlerImpl()
            keyFrameChangedHandler.handleEvent = _this.onKeyFrameChanged.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnKeyFrameChanged", keyFrameChangedHandler)

            let layerUpdatedHandler = new Module.ScriptEventHandlerImpl()
            layerUpdatedHandler.handleEvent = _this.onLayerUpdated.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnLayerUpdated", layerUpdatedHandler)

            let shapeRemovedHander = new Module.ScriptEventHandlerImpl()
            shapeRemovedHander.handleEvent = _this.onShapeRemoved.bind(_this)
            huahuoEngine.GetInstance().RegisterEvent("OnShapeRemoved", shapeRemovedHander)
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

    onKeyFrameChanged(args){
        let keyframeChangedArgs = Module.wrapPointer(args, Module.KeyFrameChangedEventHandlerArgs)
        let layer = keyframeChangedArgs.GetLayer()
        let frameId = keyframeChangedArgs.GetFrameId()

        this.timeline.redrawCell(layer, frameId)
    }

    onLayerUpdated(args){
        let layerUpdatedArgs = Module.wrapPointer(args, Module.LayerUpdatedEventHandlerArgs)
        let layer = layerUpdatedArgs.GetLayer()

        this.layerShapesManager.updateLayerShapes(layer)
    }

    onShapeRemoved(args){
        let shapeRemovedArgs = Module.wrapPointer(args, Module.ShapeRemovedEventHandlerArgs)
        let layer = shapeRemovedArgs.GetLayer()
        let obj = shapeRemovedArgs.GetShape()

        this.layerShapesManager.removeShape(layer, obj)
    }
}

export {EditorPlayer}