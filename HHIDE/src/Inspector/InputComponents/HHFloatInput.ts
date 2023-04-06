import {CustomElement} from "hhcommoncomponents";
import {HHCurveInput} from "./HHCurveInput";

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
    constructor(getter, setter, keyFrameCurveGetter, type:string = "number") {
        super();

        this.keyFrameCurveGetter = keyFrameCurveGetter
        this.getter = getter
        this.setter = setter

        this.inputElement = document.createElement("input")
        this.inputElement.type = type

        this.inputElement.style.width = "50px"
        this.inputElement.addEventListener("change", this.inputValueChanged.bind(this))
        this.appendChild(this.inputElement)

        this.curveButton = document.createElement("button")
        this.appendChild(this.curveButton)

        this.curveInput = new HHCurveInput()
        this.appendChild(this.curveInput)

        this.hideCurveInput()
    }

    get value(){
        return this.inputElement.value
    }

    set value(val){
        this.inputElement.value = val
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
        if(this.setter)
            this.setter(Number(this.inputElement.value))
    }

    hideCurveInput(){
        this.curveInput.style.display = "none"
        this.curveButton.innerText = "V"
        this.curveButton.onclick = this.showCurveInput.bind(this)
    }

    showCurveInput(){
        this.curveInput.style.display = "block"
        this.curveButton.innerText = "^"
        this.curveButton.onclick = this.hideCurveInput.bind(this)
    }

    refresh(){
        this.inputElement.value = this.getter()
    }
}

export {HHFloatInput}