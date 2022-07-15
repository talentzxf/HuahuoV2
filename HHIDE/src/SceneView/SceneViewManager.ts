import {SceneView} from "./SceneView";

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
}

let sceneViewManager = new SceneViewManager()
export {sceneViewManager}