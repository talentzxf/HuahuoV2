import {UndoableCommand} from "./UndoManager";
import {sceneViewManager} from "../SceneView/SceneViewManager";

class SetFrameIdCommand extends UndoableCommand{
    player
    prevFrameId
    currentFrameId

    constructor(player, prevFrameId, currentFrameId) {
        super();
        this.player = player
        this.prevFrameId = prevFrameId
        this.currentFrameId = currentFrameId
    }

    GetType(): string {
        return "SetFrameIdCommand";
    }

    _DoCommand() {
        sceneViewManager.focusSceneView(this.player.sceneView)
        this.player.setFrameId(this.currentFrameId)
    }

    _UnDoCommand() {
        this.player.setFrameId(this.prevFrameId)
    }
}
export {SetFrameIdCommand}