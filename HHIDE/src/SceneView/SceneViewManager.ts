import {SceneView} from "./SceneView";

class SceneViewManager{
    // Map from storeId->SceneView
    sceneViews: Map<number, SceneView> = new Map()
    curSelectedSceneView: SceneView = null

    registerSceneView(sceneView: SceneView){
        this.sceneViews.set(sceneView.storeId, sceneView)
    }

    getSceneView(storeId:number){
        if(this.sceneViews.has(storeId)){
            return this.sceneViews.get(storeId)
        }
        return null
    }
}

let sceneViewManager = new SceneViewManager()
export {sceneViewManager}