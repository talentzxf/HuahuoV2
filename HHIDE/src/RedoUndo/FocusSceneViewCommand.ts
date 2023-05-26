import {UndoableCommand, undoManager} from "./UndoManager";
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {SceneView} from "../SceneView/SceneView";
import {findParentPanel, findParentContent} from "hhpanel"
import {huahuoEngine} from "hhenginejs";

class FocusSceneViewCommand extends UndoableCommand {
    prevSceneView
    currentSceneView

    constructor(prevSceneView, currentSceneView) {
        super();
        this.prevSceneView = prevSceneView
        this.currentSceneView = currentSceneView
    }

    GetType(): string {
        return "SwitchSceneView";
    }

    selectAndFocusSceneView(sceneView: SceneView) {
        // Find the panel
        let panel = findParentPanel(sceneView)
        let content = findParentContent(sceneView)
        // Get Index of the content in the panel
        let viewIndex = content.getTitle().tabIndex
        panel.selectTab(viewIndex)

        sceneViewManager.focusSceneView(sceneView, false)
    }

    _DoCommand() {
        this.selectAndFocusSceneView(this.currentSceneView)
    }

    _UnDoCommand() {
        if (this.prevSceneView != null)
            this.selectAndFocusSceneView(this.prevSceneView)
    }
}

// Have to use this ugly event pubsub to break the cyclic dependency loop.
huahuoEngine.registerEventListener("HHIDE", "PushFocusSceneViewCommand", (previousSceneView, curFocusedSceneView) => {
    undoManager.PushCommand(new FocusSceneViewCommand(previousSceneView, curFocusedSceneView))
})

export {FocusSceneViewCommand}