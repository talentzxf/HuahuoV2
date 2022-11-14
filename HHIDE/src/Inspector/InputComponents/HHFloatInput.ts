import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-float-input",
    extends: "input"
})
class HHFloatInput extends HTMLInputElement{
}

export {HHFloatInput}