import {SceneView} from "./SceneView";
import {Logger} from "hhcommoncomponents"
import {Player} from "hhenginejs"

class SceneViewManager{
    // Map from storeId->SceneView
    sceneViews: Map<number, SceneView> = new Map()
    curFocusedSceneView: SceneView = null

    registerSceneView(sceneView: SceneView){
        this.sceneViews.set(sceneView.storeId, sceneView)
    }

    getSceneView(storeId:number){
        if(this.sceneViews.has(storeId)){
            return this.sceneViews.get(storeId)
        }
        return null
    }

    focusSceneView(sceneView:SceneView){
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