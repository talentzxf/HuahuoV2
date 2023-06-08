import {MergableCommand} from "../UndoManager";
class SetFloatCommand extends MergableCommand{
    setter: Function
    oldValue: number
    newValue: number

    constructor(setter, oldValue, newValue) {
        super();

        this.setter = setter
        this.oldValue = oldValue
        this.newValue = newValue
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
        if(newSetFloatValueCommand.setter == this.setter){
            this.newValue = newSetFloatValueCommand.newValue
            return true
        }

        return false
    }
}

export {SetFloatCommand}