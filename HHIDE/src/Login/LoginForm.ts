import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-login-form"
})
class LoginForm extends HTMLElement {
    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        let span = document.createElement("span")
        span.innerText = "Login!!!!!"
        this.appendChild(span)
    }
}

let loginForm = null;

function openLoginForm() {
    if(loginForm == null){
        loginForm = document.createElement("hh-login-form")
        document.body.appendChild(loginForm)
    }


}
export {openLoginForm}