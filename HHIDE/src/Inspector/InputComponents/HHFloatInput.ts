import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-float-input",
    extends: "input"
})
class HHFloatInput extends HTMLInputElement implements RefreshableComponent{
    getter: Function

    refresh(){
        this.value = this.getter()
    }
}

export {HHFloatInput}