import {MergableCommand} from "./UndoManager";

class SetFieldValueCommand<ValueType> extends MergableCommand {
    setter: Function
    oldValue: ValueType
    newValue: ValueType

    constructor(setter, oldValue: ValueType, newValue: ValueType) {
        super();

        this.setter = setter
        this.oldValue = oldValue
        this.newValue = newValue

    }

    GetType(): string {
        return "SetValue From:" + this.oldValue + " to" + this.newValue;
    }

    _DoCommand() {
        this.setter(this.newValue)
    }

    _UnDoCommand() {
        this.setter(this.oldValue)
    }

    override MergeCommand(anotherCommand: MergableCommand): boolean {
        let newSetFloatValueCommand = anotherCommand as SetFieldValueCommand<ValueType>
        if (newSetFloatValueCommand.setter == this.setter) {
            this.newValue = newSetFloatValueCommand.newValue
            return true
        }

        return false
    }
}

export {SetFieldValueCommand}