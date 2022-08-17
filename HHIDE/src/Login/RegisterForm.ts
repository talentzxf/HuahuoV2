import {CustomElement} from "hhcommoncomponents"

@CustomElement({
    selector: "hh-register-form"
})
class RegisterForm extends HTMLElement {
    connectedCallback(){
        this.innerHTML = "Please register!!!"
    }
}

export {RegisterForm}