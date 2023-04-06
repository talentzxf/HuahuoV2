import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-float-input"
})
class HHFloatInput extends HTMLElement implements RefreshableComponent{
    getter: Function
    setter: Function

    inputElement: HTMLInputElement
    constructor(getter, setter, type:string = "number") {
        super();

        this.getter = getter
        this.setter = setter

        this.inputElement = document.createElement("input")
        this.inputElement.type = type

        this.inputElement.style.width = "50px"
        this.inputElement.addEventListener("change", this.inputValueChanged.bind(this))
        this.appendChild(this.inputElement)
    }

    inputValueChanged(){
        if(this.setter)
            this.setter(Number(this.inputElement.value))
    }

    connectedCallback(){
        this.refresh()

        let curveButton = document.createElement("button")
        curveButton.innerHTML = "V"
        this.appendChild(curveButton)
    }

    refresh(){
        this.inputElement.value = this.getter()
    }
}

export {HHFloatInput}