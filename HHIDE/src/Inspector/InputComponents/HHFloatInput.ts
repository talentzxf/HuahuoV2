import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-float-input",
    extends: "input"
})
class HHFloatInput extends HTMLInputElement implements RefreshableComponent{
    getter: Function
    setter: Function

    constructor(getter, setter, type:string = "number") {
        super();

        this.getter = getter
        this.setter = setter

        this.style.width = "50px"
        this.type = type
        this.addEventListener("change", this.inputValueChanged.bind(this))
    }

    inputValueChanged(){
        if(this.setter)
            this.setter(Number(this.value))
    }

    connectedCallback(){
        this.refresh()
    }

    refresh(){
        this.value = this.getter()
    }
}

export {HHFloatInput}