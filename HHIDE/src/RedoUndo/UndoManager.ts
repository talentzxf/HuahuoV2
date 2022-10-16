import {huahuoEngine} from "hhenginejs";
import {sceneViewManager} from "../SceneView/SceneViewManager";

class ExecutionStackFrame{
    private store
    private layer
    private frameId
    private player

    constructor() {
        this.store = huahuoEngine.GetCurrentStore()
        this.layer = huahuoEngine.GetCurrentLayer()
        this.frameId = this.layer.GetCurrentFrame()
        this.player = sceneViewManager.getFocusedViewAnimationPlayer()
    }

    restore(){
        this.layer.SetCurrentFrame(this.frameId)
        this.store.SetCurrentLayer(this.layer)
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.store.GetStoreId())

        this.player.updateAllShapes()
    }

    equals(otherFrame: ExecutionStackFrame){
        return this.store == otherFrame.store && this.frameId == otherFrame.frameId && this.store == otherFrame.store
    }
}

abstract class UndoableCommand {
    abstract GetType(): string
    abstract _DoCommand()
    abstract _UnDoCommand()

    private stackFrame: ExecutionStackFrame
    constructor() {
        this.stackFrame = new ExecutionStackFrame()
    }

    stackFrameEqual(anotherCommand: UndoableCommand):boolean{
        return this.stackFrame.equals(anotherCommand.stackFrame)
    }

    private ExecuteCommand(func){
        // let stackFrame: ExecutionStackFrame = new ExecutionStackFrame()
        // try{
        //     this.stackFrame.restore() // Return to the status when the command is created.
        //     func()
        // }finally {
        //     stackFrame.restore()
        // }

        func()
    }

    public DoCommand(){
        this.ExecuteCommand(this._DoCommand.bind(this))
    }

    public UnDoCommand(){
        this.ExecuteCommand(this._UnDoCommand.bind(this))
    }
}

abstract class MergableCommand extends UndoableCommand{
    abstract MergeCommand(anotherCommand:MergableCommand): boolean
}

function commandIsMergable(cmd:UndoableCommand){
    return (<MergableCommand>cmd).MergeCommand !== undefined
}

class UndoManager {
    undoCommandStack: Array<UndoableCommand> = new Array<UndoableCommand>()

    currentCmdIdx: number = -1 // -1 means currently no command.

    isValidIndex(idx):boolean{
        return idx >= 0 && idx <= this.undoCommandStack.length - 1
    }

    getUndoIndex(){
        if(this.undoCommandStack.length == 0)
            return -1
        if(this.currentCmdIdx == this.undoCommandStack.length)
            return this.currentCmdIdx

        return this.currentCmdIdx
    }

    PushCommand(cmd: UndoableCommand) {
        // Discard all commands behind current index
        while (this.currentCmdIdx != this.undoCommandStack.length - 1
            &&this.undoCommandStack.length != 0) {
            this.undoCommandStack.pop()
        }

        let undoCmdIdx = this.getUndoIndex()

        if(this.isValidIndex(undoCmdIdx)){
            let currentDoneCommand = this.undoCommandStack[undoCmdIdx]
            if(commandIsMergable(currentDoneCommand) && commandIsMergable(cmd)){
                if( (currentDoneCommand as MergableCommand).MergeCommand(cmd as MergableCommand) ){ // The new command is merged into previous command.
                    return
                }
            }
        }

        this.undoCommandStack.push(cmd)
        this.currentCmdIdx = this.undoCommandStack.length - 1
    }

    UnDo() {
        if(this.currentCmdIdx == this.undoCommandStack.length){ // At the top, can only undo the last command.
            this.currentCmdIdx = Math.max(this.currentCmdIdx - 1 , -1 )
        }

        if(this.isValidIndex(this.currentCmdIdx)){
            let currentCommand = this.undoCommandStack[this.currentCmdIdx]
            currentCommand.UnDoCommand()
        }
        this.currentCmdIdx = Math.max(this.currentCmdIdx - 1 , -1 )
    }

    ReDo() {
        this.currentCmdIdx = Math.min(this.currentCmdIdx + 1 , this.undoCommandStack.length)
        if(this.isValidIndex(this.currentCmdIdx)){
            let currentCommand = this.undoCommandStack[this.currentCmdIdx]
            currentCommand.DoCommand()
        }
    }
}

let undoManager = new UndoManager()

export {UndoableCommand, MergableCommand, ExecutionStackFrame, undoManager}