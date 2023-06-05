import {UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";
import {AbstractComponent} from "hhenginejs";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";

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
        // Add the component back.
        this.targetShape.addComponent(this.targetComponent)
        IDEEventBus.getInstance().emit(EventNames.COMPONENTCHANGED, this.targetShape)
    }

    _UnDoCommand() {
        this.targetComponent.detachFromCurrentShape()

        IDEEventBus.getInstance().emit(EventNames.COMPONENTCHANGED, this.targetShape)
    }

}

export {AddComponentCommand}