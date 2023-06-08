import {MergableCommand} from "../UndoManager";
import {AbstractComponent} from "hhenginejs";
class SetFloatCommand extends MergableCommand{
    setter: Function
    oldValue: number
    newValue: number

    targetComponent: AbstractComponent

    constructor(setter, oldValue, newValue, targetComponent = null) {
        super();

        this.setter = setter
        this.oldValue = oldValue
        this.newValue = newValue

        this.targetComponent = targetComponent
    }
    GetType(): string {
        return "SetFloatValue From:" + this.oldValue + " to" + this.newValue;
    }

    _DoCommand() {
        this.setter(this.newValue)
    }

    _UnDoCommand() {
        this.setter(this.oldValue)
    }

    override MergeCommand(anotherCommand:MergableCommand): boolean{
        let newSetFloatValueCommand = anotherCommand as SetFloatCommand
        if(newSetFloatValueCommand.setter == this.setter || newSetFloatValueCommand.targetComponent == this.targetComponent){
            this.newValue = newSetFloatValueCommand.newValue
            return true
        }

        return false
    }
}

export {SetFloatCommand}