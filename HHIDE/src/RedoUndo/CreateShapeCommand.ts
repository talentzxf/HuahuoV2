import {UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";

class CreateShapeCommand extends UndoableCommand{
    private targetShape: BaseShapeJS
    private layer

    constructor(layer, targetShape: BaseShapeJS) {
        super()
        this.layer = layer
        this.targetShape = targetShape
    }

    _DoCommand() { // Add the shape to layer.
        this.layer.addShape(this.targetShape)
    }

    _UnDoCommand() {
        this.targetShape.remove()
    }

    GetType(): string {
        return "CreateShape";
    }
}

export {CreateShapeCommand}