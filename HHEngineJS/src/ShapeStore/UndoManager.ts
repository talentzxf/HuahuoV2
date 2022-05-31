interface UndoableCommand{
    DoCommand()
    UnDoCommand()
}

class UndoManager{
    undoCommandStack: Array<UndoableCommand> = new Array<UndoableCommand>()

    currentCmdIdx: number

    PushCommand(cmd:UndoableCommand){
        this.undoCommandStack.push(cmd)
        this.currentCmdIdx = this.undoCommandStack.length - 1
    }
}