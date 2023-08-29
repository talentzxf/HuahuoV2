import {HHTimeLine, TimelineEventNames} from "hhtimeline"
import {huahuoEngine, GlobalConfig} from "hhenginejs"
import {Player} from "hhenginejs/src/Player/Player"; // Not sure what's the best way to import a class and extend it in another package....
import {SceneView} from "../SceneView/SceneView";
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {undoManager} from "../RedoUndo/UndoManager";
import {SetFrameIdCommand} from "../RedoUndo/SetFrameIdCommand";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";

declare var Module: any;

class EditorPlayer extends Player {

    timeline: HHTimeLine = null
    sceneView: SceneView = null

    constructor(sceneView) {
        super(sceneView.storeId)

        this.sceneView = sceneView

        this.timeline = sceneView.timeline
        this.timeline.addEventListener(TimelineEventNames.TRACKCELLCLICKED, this.onSetFrameTime.bind(this))

        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            let keyFrameChangedHandler = new Module.ScriptEventHandlerImpl()
            keyFrameChangedHandler.handleEvent = _this.onKeyFrameChanged.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnKeyFrameChanged", keyFrameChangedHandler)

            let tryAddKeyFrameHandler = new Module.ScriptEventHandlerImpl()
            tryAddKeyFrameHandler.handleEvent = _this.onTryAddKeyFrame.bind(_this)
            huahuoEngine.GetInstance().RegisterEvent("TryAddKeyFrame", tryAddKeyFrameHandler)

            let layerUpdatedHandler = new Module.ScriptEventHandlerImpl()
            layerUpdatedHandler.handleEvent = _this.onLayerUpdated.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnLayerUpdated", layerUpdatedHandler)

            let shapeRemovedHander = new Module.ScriptEventHandlerImpl()
            shapeRemovedHander.handleEvent = _this.onShapeRemoved.bind(_this)
            huahuoEngine.GetInstance().RegisterEvent("OnShapeRemoved", shapeRemovedHander)

            let maxFrameIdUpdatedHandler = new Module.ScriptEventHandlerImpl()
            maxFrameIdUpdatedHandler.handleEvent = _this.onMaxFrameUpdated.bind(_this)
            huahuoEngine.GetInstance().RegisterEvent("OnMaxFrameIdUpdated", maxFrameIdUpdatedHandler)

            IDEEventBus.getInstance().on(EventNames.FILELOADED, _this.onFileLoaded.bind(_this))
        })
    }

    onSetFrameTime(e) {
        sceneViewManager.focusSceneView(this.sceneView)

        // Set current store
        let currentStoreId = this.sceneView.storeId
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(currentStoreId)

        let elapsedTime = e.detail.elapsedTime
        let frameId = Math.floor(elapsedTime * GlobalConfig.fps)
        this.setFrameId(frameId)

        let prevTime = e.detail.prevTime
        let prevFrameId = Math.floor(prevTime * GlobalConfig.fps)
        if (prevFrameId != frameId)
            undoManager.PushCommand(new SetFrameIdCommand(this.sceneView.animationPlayer, prevFrameId, frameId))
    }

    setFrameId(playFrameId, force = false) {
        if (sceneViewManager.getFocusedSceneView() != this.sceneView && force == false) {
            this.pausePlay() // Lost focus, stop play
        } else {
            super.setFrameId(playFrameId)

            playFrameId += 0.5  // Force to start at 1 for better visualization
            this.timeline.setTimeElapsed(playFrameId / GlobalConfig.fps)

            IDEEventBus.getInstance().emit(EventNames.CELLCLICKED, playFrameId)
        }
    }

    // If something is changed, TryAddKeyFrame event will be triggered.
    onTryAddKeyFrame(args){
        let layerUpdatedEventArgs = Module.wrapPointer(args, Module.LayerUpdatedEventHandlerArgs)
        let layer = layerUpdatedEventArgs.GetLayer()
        if(layer != null){
            this.timeline.selectLayer(layer)

            this.timeline.redrawCanvas()
        }
    }

    onKeyFrameChanged(args) {
        let keyframeChangedArgs = Module.wrapPointer(args, Module.KeyFrameChangedEventHandlerArgs)
        let layer = keyframeChangedArgs.GetLayer()

        // let frameId = keyframeChangedArgs.GetFrameId()

        // Check if this event belongs to this EditorPlayer.
        if (this.sceneView.storeId == layer.GetObjectStore().GetStoreId()){
            let maxFrameId = layer.GetObjectStore().GetMaxFrameId()
            if (maxFrameId >= 0) {
                this.timeline.setMaxCellId(maxFrameId + 1)
            }

            // this.timeline.redrawCell(layer, frameId)
            this.timeline.redrawCanvas()
        }
    }

    onLayerUpdated(args) {
        let layerUpdatedArgs = Module.wrapPointer(args, Module.LayerUpdatedEventHandlerArgs)
        let layer = layerUpdatedArgs.GetLayer()

        this.layerShapesManager.updateLayerShapes(layer)
    }

    onShapeRemoved(args) {
        let shapeRemovedArgs = Module.wrapPointer(args, Module.ShapeRemovedEventHandlerArgs)
        let layer = shapeRemovedArgs.GetLayer()
        let obj = shapeRemovedArgs.GetShape()

        this.layerShapesManager.removeShape(layer, obj)
    }


    onMaxFrameUpdated() {
        let storeId = this.sceneView.storeId
        let store = huahuoEngine.GetStoreById(storeId)
        let maxFrameId = store.GetMaxFrameId()
        if (maxFrameId >= 0) {
            this.timeline.setMaxCellId(maxFrameId + 1)
        }

        this.timeline.redrawCanvas()
    }

    onFileLoaded(fileName: string){
        this.onMaxFrameUpdated()
    }
}

export {EditorPlayer}