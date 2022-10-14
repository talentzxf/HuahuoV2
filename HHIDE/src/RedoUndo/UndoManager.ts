interface UndoableCommand {
    DoCommand()

    UnDoCommand()
}

class UndoManager {
    undoCommandStack: Array<UndoableCommand> = new Array<UndoableCommand>()

    currentCmdIdx: number = 0

    PushCommand(cmd: UndoableCommand) {
        // Discard all commands behind current index
        while (this.currentCmdIdx != this.undoCommandStack.length) {
            this.undoCommandStack.pop()
        }

        this.undoCommandStack.push(cmd)
        this.currentCmdIdx = this.undoCommandStack.length - 1
    }

    UnDo() {
        this.currentCmdIdx = Math.max(this.currentCmdIdx - 1 , 0 )

        let currentCommand = this.undoCommandStack[this.currentCmdIdx]
        currentCommand.DoCommand()
    }

    ReDo() {
        this.currentCmdIdx = Math.min(this.currentCmdIdx + 1 , this.undoCommandStack.length - 1)
        let currentCommand = this.undoCommandStack[this.currentCmdIdx]
        currentCommand.DoCommand()
    }
}

let undoManager = new UndoManager()

export {UndoableCommand, undoManager}