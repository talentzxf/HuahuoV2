import {MergableCommand, UndoableCommand} from "./UndoManager";
import {sceneViewManager} from "../SceneView/SceneViewManager";

let commandName = "SetFrameIdCommand"
class SetFrameIdCommand extends MergableCommand{
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
        return commandName;
    }

    _DoCommand() {
        sceneViewManager.focusSceneView(this.player.sceneView)
        this.player.setFrameId(this.currentFrameId)
    }

    _UnDoCommand() {
        this.player.setFrameId(this.prevFrameId)
    }

    MergeCommand(anotherCommand: MergableCommand): boolean {
        if (anotherCommand.GetType() == commandName) {
            let setFrameIdCommand = anotherCommand as SetFrameIdCommand
            if (setFrameIdCommand) {
                if(this.player == setFrameIdCommand.player){

                    this.currentFrameId = setFrameIdCommand.currentFrameId
                    return true
                }
            }
        }
        return false;
    }
}
export {SetFrameIdCommand}