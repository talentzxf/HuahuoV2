interface UndoableCommand {
    DoCommand()

    UnDoCommand()
}

class UndoManager {
    undoCommandStack: Array<UndoableCommand> = new Array<UndoableCommand>()

    currentCmdIdx: number = -1 // -1 means currently no command.

    isValidIndex(idx):boolean{
        return idx >= 0 && idx <= this.undoCommandStack.length - 1
    }

    PushCommand(cmd: UndoableCommand) {
        // Discard all commands behind current index
        while (this.currentCmdIdx != this.undoCommandStack.length - 1) {
            this.undoCommandStack.pop()
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

export {UndoableCommand, undoManager}