import {UndoableCommand} from "./UndoManager";
import {ArrayInsertCommand} from "./ArrayInsertCommand";

class ArrayDeleteCommand extends UndoableCommand{

    arrayInsertCommand: ArrayInsertCommand

    constructor(inserter, deleter) {
        super();

        this.arrayInsertCommand = new ArrayInsertCommand(inserter, deleter)
    }
    GetType(): string {
        return "ArrayInsertCommand";
    }

    _DoCommand() {
        this.arrayInsertCommand.UnDoCommand()
    }

    _UnDoCommand() {
        this.arrayInsertCommand.DoCommand()
    }

}

export {ArrayDeleteCommand}