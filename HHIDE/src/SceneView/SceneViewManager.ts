import {SceneView} from "./SceneView";
import {Logger} from "hhcommoncomponents"
import {Player} from "hhenginejs"
import {huahuoEngine} from "hhenginejs";

class SceneViewManager{
    // Map from storeId->SceneView
    sceneViews: Map<number, SceneView> = new Map()
    curFocusedSceneView: SceneView = null

    // Map from sceneview to frameId, when switch back to this scene, we need to restore it's frameId.
    sceneViewFrameIdMap: Map<SceneView, number> = new Map()

    registerSceneView(sceneView: SceneView){
        this.sceneViews.set(sceneView.storeId, sceneView)
    }

    getSceneViewFrameId(sceneView: SceneView){
        let storeId = sceneView.storeId
        return huahuoEngine.GetStoreById(storeId).GetCurrentLayer().GetCurrentFrame()
    }

    getSceneView(storeId:number){
        if(this.sceneViews.has(storeId)){
            return this.sceneViews.get(storeId)
        }
        return null
    }

    setStoreFrameId(storeId, frameId){
        let store = huahuoEngine.GetStoreById(storeId)
        let layerCount = store.GetLayerCount()
        for(let layerIdx = 0; layerIdx < layerCount; layerIdx++){
            let targetLayer = store.GetLayer(layerIdx)
            targetLayer.SetCurrentFrame(frameId)
        }
    }

    focusSceneView(sceneView:SceneView){
        // save the currently focused scene view frameId.
        if(this.curFocusedSceneView){
            this.sceneViewFrameIdMap.set(this.curFocusedSceneView, this.getSceneViewFrameId(this.curFocusedSceneView))
        }

        // restore the saved frameId
        if(this.sceneViewFrameIdMap.has(sceneView)){
            this.setStoreFrameId(sceneView.storeId, this.sceneViewFrameIdMap.get(sceneView))
        }

        this.curFocusedSceneView = sceneView
    }

    getFocusedSceneView(){
        return this.curFocusedSceneView
    }

    removeSceneViewMap(storeId: number){
        this.sceneViews.delete(storeId)
    }

    getFocusedViewAnimationPlayer(): Player{
        let targetSceneView:SceneView = sceneViewManager.getFocusedSceneView()
        if(targetSceneView == null) { // Currently, no scene view is focused. But Why???
            Logger.error("No scene view is focused!")
            return null
        }

        return targetSceneView.animationPlayer
    }
}

let sceneViewManager = new SceneViewManager()
export {sceneViewManager}