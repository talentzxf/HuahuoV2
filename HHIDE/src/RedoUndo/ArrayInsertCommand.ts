import {UndoableCommand} from "./UndoManager";

class ArrayInsertCommand extends UndoableCommand{

    inserter: Function
    deleter: Function

    lastInsertedObj: Object
    constructor(inserter, deleter) {
        super();

        this.inserter = inserter
        this.deleter = deleter
    }
    GetType(): string {
        return "ArrayInsertCommand";
    }

    _DoCommand() {
        this.lastInsertedObj = this.inserter()
    }

    _UnDoCommand() {
        this.deleter(this.lastInsertedObj)
    }

}

export {ArrayInsertCommand}