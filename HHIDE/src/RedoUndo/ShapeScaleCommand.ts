import {MergableCommand, TransformCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";

let commandName = "ScaleShape"
class ShapeScaleCommand extends TransformCommand{
    targetShape: BaseShapeJS
    prevScale
    targetScale

    constructor(shape: BaseShapeJS, prevScale, targetScale) {
        super();
        this.targetShape = shape
        this.prevScale = prevScale.clone()
        this.targetScale = targetScale.clone()
    }

    GetType(): string {
        return commandName;
    }

    MergeCommand(anotherCommand: MergableCommand): boolean {
        return false;
    }

    _DoCommand() {
        this.targetShape.scaling = this.targetScale
    }

    _UnDoCommand() {
        this.targetShape.scaling = this.prevScale
    }
}

export {ShapeScaleCommand}