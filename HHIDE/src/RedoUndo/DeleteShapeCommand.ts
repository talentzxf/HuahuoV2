import {UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";
import {CreateShapeCommand} from "./CreateShapeCommand";

class DeleteShapeCommand extends UndoableCommand{

    createShapeCommand: CreateShapeCommand;

    constructor(layer, targetShape: BaseShapeJS) {
        super()
        this.createShapeCommand = new CreateShapeCommand(layer, targetShape)
    }

    GetType(): string {
        return "DeleteShapeCommand";
    }

    _DoCommand() {
        this.createShapeCommand.UnDoCommand()
    }

    _UnDoCommand() {
        this.createShapeCommand.DoCommand()
    }
}

export {DeleteShapeCommand}