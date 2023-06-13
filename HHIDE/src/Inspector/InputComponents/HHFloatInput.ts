import {CustomElement} from "hhcommoncomponents";
import {HHCurveInput} from "./HHCurveInput";
import {undoManager} from "../../RedoUndo/UndoManager";
import {SetFieldValueCommand} from "../../RedoUndo/SetFieldValueCommand";

@CustomElement({
    selector: "hh-float-input"
})
class HHFloatInput extends HTMLElement implements RefreshableComponent{
    getter: Function
    setter: Function
    keyFrameCurveGetter: Function

    curveButton: HTMLButtonElement
    curveInput: HHCurveInput

    inputElement: HTMLInputElement
    constructor(getter, setter, keyFrameCurveGetter = null, type:string = "number") {
        super();

        this.keyFrameCurveGetter = keyFrameCurveGetter
        this.getter = getter
        this.setter = setter

        this.inputElement = document.createElement("input")
        this.inputElement.className = "form-control"
        this.inputElement.type = type

        this.inputElement.addEventListener("change", this.inputValueChanged.bind(this))
        this.appendChild(this.inputElement)

        this.className = "input-group"

        if(this.keyFrameCurveGetter){
            this.curveButton = document.createElement("button")
            this.curveButton.className = "btn btn-outline-secondary"
            this.appendChild(this.curveButton)

            this.curveInput = new HHCurveInput(this.keyFrameCurveGetter)
            this.appendChild(this.curveInput)

            this.hideCurveInput()
        }
    }

    get value(){
        return this.inputElement.value
    }

    set value(val){
        this.inputElement.value = Number.parseFloat(val).toFixed(2)
        if(this.curveInput){
            this.curveInput.refresh()
        }
    }

    set min(val){
        this.inputElement.min = val
    }

    set max(val){
        this.inputElement.max = val
    }

    set step(val){
        this.inputElement.step = val
    }

    connectedCallback(){
        this.refresh()
    }

    inputValueChanged(){
        if(this.setter){
            let oldValue = Number(this.getter())
            let newValue = Number(this.inputElement.value)

            let command = new SetFieldValueCommand<number>(this.setter, oldValue, newValue)
            undoManager.PushCommand(command)
            command.DoCommand()
        }
    }

    hideCurveInput(){
        if(this.keyFrameCurveGetter){
            this.curveInput.style.display = "none"
            this.curveButton.innerText = "V"
            this.curveButton.onclick = this.showCurveInput.bind(this)
        }
    }

    showCurveInput(){
        if(this.keyFrameCurveGetter){
            this.curveInput.style.display = "block"
            this.curveButton.innerText = "^"
            this.curveButton.onclick = this.hideCurveInput.bind(this)
            this.curveInput.refresh()
        }
    }

    refresh(){
        this.inputElement.value = Number.parseFloat(this.getter()).toFixed(2)

        if(this.curveInput){
            this.curveInput.refresh()
        }
    }
}

export {HHFloatInput}