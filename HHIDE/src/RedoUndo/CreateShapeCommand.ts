import {UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";

class CreateShapeCommand implements UndoableCommand{
    private targetShape: BaseShapeJS
    private layer

    constructor(layer, targetShape: BaseShapeJS) {
        this.layer = layer
        this.targetShape = targetShape
    }

    DoCommand() { // Add the shape to layer.
        this.layer.addShape(this.targetShape)
    }

    UnDoCommand() {
        this.targetShape.detachFromCurrentLayer()
        this.targetShape.removePaperObj()
    }

    GetType(): string {
        return "CreateShape";
    }
}

export {CreateShapeCommand}