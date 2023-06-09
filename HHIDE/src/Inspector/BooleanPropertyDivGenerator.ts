import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../RedoUndo/UndoManager";

class BooleanPropertyDesc extends BasePropertyDesc{
    getter
    setter

    checkBox: HTMLInputElement
    constructor(property: Property) {
        super(property);
        this.checkBox = document.createElement("input")
        this.checkBox.type = "checkbox"

        this.setter = property.setter
        this.getter = property.getter

        this.checkBox.checked = this.getter()

        this.checkBox.addEventListener("change", this.inputValueChanged.bind(this))

        this.contentDiv.appendChild(this.checkBox)
    }

    inputValueChanged(){
        let oldValue = this.getter()
        let newValue = this.checkBox.checked

        let setBooleanValueCommand = new SetFieldValueCommand(this.setter, oldValue, newValue)
        setBooleanValueCommand.DoCommand()
        undoManager.PushCommand(setBooleanValueCommand)
    }
}

class BooleanPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        let booleanPropertyDesc = new BooleanPropertyDesc(property)
        return booleanPropertyDesc;
    }
}

let booleanPropertyDivGenerator = new BooleanPropertyDivGenerator()
export {booleanPropertyDivGenerator}