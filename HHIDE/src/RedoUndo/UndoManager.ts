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

    }

    ReDo() {

    }
}

let undoManager = new UndoManager()

export {UndoableCommand, undoManager}