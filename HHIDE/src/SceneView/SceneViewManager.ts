import {SceneView} from "./SceneView";
import {Logger} from "hhcommoncomponents"
import {Player} from "hhenginejs"
import {huahuoEngine, renderEngine2D} from "hhenginejs";
import {undoManager} from "../RedoUndo/UndoManager";
import {FocusSceneViewCommand} from "../RedoUndo/FocusSceneViewCommand";

class SceneViewManager {
    // Map from storeId->SceneView
    sceneViews: Map<string, SceneView> = new Map()
    curFocusedSceneView: SceneView = null

    // Map from sceneview to frameId, when switch back to this scene, we need to restore it's frameId.
    sceneViewFrameIdMap: Map<SceneView, number> = new Map()

    registerSceneView(sceneView: SceneView) {
        this.sceneViews.set(sceneView.storeId, sceneView)
    }

    getSceneViewFrameId(sceneView: SceneView) {
        let storeId = sceneView.storeId
        return huahuoEngine.GetStoreById(storeId).GetCurrentLayer().GetCurrentFrame()
    }

    getSceneView(storeId: string) {
        if (this.sceneViews.has(storeId)) {
            return this.sceneViews.get(storeId)
        }
        return null
    }

    focusSceneView(sceneView: SceneView, pushCommand = true) {
        // Do nothing if the scene has already been focused.
        if (sceneView == this.curFocusedSceneView)
            return

        let previousSceneView = this.curFocusedSceneView

        // save the currently focused scene view frameId.
        if (this.curFocusedSceneView && this.curFocusedSceneView.animationPlayer != null) {
            this.sceneViewFrameIdMap.set(this.curFocusedSceneView, this.curFocusedSceneView.animationPlayer.currentlyPlayingFrameId)
        }

        // restore the saved frameId
        if (this.sceneViewFrameIdMap.has(sceneView)) {
            sceneView.animationPlayer.setFrameId(this.sceneViewFrameIdMap.get(sceneView), true, true)
        }

        this.curFocusedSceneView = sceneView

        let canvas = sceneView.querySelector(".SceneViewCanvas")
        renderEngine2D.setDefaultCanvas(canvas)

        let player = sceneView.animationPlayer
        huahuoEngine.setActivePlayer(player)
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(sceneView.storeId)
        player.updateAllShapes()

        sceneView.resetDefaultShapeDrawer()

        if (pushCommand && previousSceneView != null) {
            // Have to use event to push the command, or else, there will be a cyclic dependency loop.
            huahuoEngine.dispatchEvent("HHIDE", "PushFocusSceneViewCommand", previousSceneView, this.curFocusedSceneView)
        }

    }

    getFocusedSceneView() {
        return this.curFocusedSceneView
    }

    removeSceneViewMap(storeId: string) {
        this.sceneViews.delete(storeId)
    }

    getFocusedViewAnimationPlayer(): Player {
        let targetSceneView: SceneView = sceneViewManager.getFocusedSceneView()
        if (targetSceneView == null) { // Currently, no scene view is focused. But Why???
            Logger.error("No scene view is focused!")
            return null
        }

        return targetSceneView.animationPlayer
    }
}

let sceneViewManager = new SceneViewManager()
export {sceneViewManager}