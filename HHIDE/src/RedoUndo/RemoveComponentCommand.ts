import {AddComponentCommand} from "./AddComponentCommand";
import {UndoableCommand, undoManager} from "./UndoManager";
import {AbstractComponent, BaseShapeJS, huahuoEngine} from "hhenginejs";


// Remove component is the reverse of the AddComponentCommand
class RemoveComponentCommand extends UndoableCommand{
    addComponentCommand: AddComponentCommand
    constructor(targetComponent:AbstractComponent) {
        super();
        this.addComponentCommand = new AddComponentCommand(targetComponent)
    }
    GetType(): string {
        return "RemoveComponentCommand";
    }

    _DoCommand() {
        this.addComponentCommand.UnDoCommand()
    }

    _UnDoCommand() {
        this.addComponentCommand.DoCommand()
    }
}


// Have to use this ugly event pubsub to break the cyclic dependency loop.
huahuoEngine.registerEventListener("HHIDE", "DeleteComponent", (component) => {
    let removeComponentCommand = new RemoveComponentCommand(component)
    undoManager.PushCommand(removeComponentCommand)
    removeComponentCommand.DoCommand()
})

export {RemoveComponentCommand}