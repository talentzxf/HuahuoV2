import {UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";
import {AbstractComponent} from "hhenginejs/dist/src/Components/AbstractComponent";

class AddComponentCommand extends UndoableCommand{

    targetComponent: AbstractComponent
    targetShape: BaseShapeJS

    constructor(targetShape: BaseShapeJS, targetComponent:AbstractComponent) {
        super();

        this.targetComponent = targetComponent
        this.targetShape = targetShape
    }

    GetType(): string {
        return "AddComponentCommand";
    }

    _DoCommand() {
    }

    _UnDoCommand() {
        this.targetComponent.detachFromCurrentShape()

    }

}

export {AddComponentCommand}