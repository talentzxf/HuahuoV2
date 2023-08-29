import {UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";
import {AbstractComponent} from "hhenginejs";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {elementCreator} from "../SceneView/ElementCreator";

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
        this.targetComponent.enableComponent()
        this.targetComponent.onMounted()
        elementCreator.dispatchElementChange(this.targetShape.belongStoreId)

        IDEEventBus.getInstance().emit(EventNames.COMPONENTCHANGED, this.targetShape)
    }

    _UnDoCommand() {
        // Remove the component.
        this.targetComponent.detachFromCurrentShape()
        this.targetComponent.disableComponent()
        this.targetComponent.onDismounted()
        elementCreator.dispatchElementChange(this.targetShape.belongStoreId)

        IDEEventBus.getInstance().emit(EventNames.COMPONENTCHANGED, this.targetShape)
    }

}

export {AddComponentCommand}