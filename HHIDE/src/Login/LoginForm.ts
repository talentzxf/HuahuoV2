import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-login-form"
})
class LoginForm extends HTMLElement {
    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        let container = document.createElement("form")
        container.innerHTML = "" +
            "<div class=container>" +
            "   <label for='uname'><b>Username</b></label>" +
            "   <input type='text' placeholder='Enter Username' name='username'> " +
            "</div>"

        this.appendChild(container)
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