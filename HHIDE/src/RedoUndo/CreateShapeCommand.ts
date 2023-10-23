import {UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";

class CreateShapeCommand extends UndoableCommand {
    private targetShape: BaseShapeJS
    private layer

    constructor(layer, targetShape: BaseShapeJS) {
        super()
        this.layer = layer
        this.targetShape = targetShape
    }

    _DoCommand() { // Add the shape to layer.
        this.layer.addShape(this.targetShape)
        IDEEventBus.getInstance().emit(EventNames.OBJECTADDED, this.targetShape)
    }

    _UnDoCommand() {
        this.targetShape.remove()
    }

    GetType(): string {
        return "CreateShape";
    }
}

export {CreateShapeCommand}