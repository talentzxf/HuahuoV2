import {CustomElement} from "hhcommoncomponents"
import {LoginForm} from "./LoginForm";

@CustomElement({
    selector: "hh-register-form"
})
class RegisterForm extends HTMLElement {
    closeBtn: HTMLButtonElement
    form: HTMLFormElement

    connectedCallback(){
        this.innerHTML =
            "   <form id='registerForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='registerFormCloseBtn' >" +
            "           <img class='far fa-circle-xmark'></img>" +
            "       </div>" +
            "   </div>" +
            "       <h3>Register Here</h3>" +
            "       <label for='uname'><b>Username</b></label>" +
            "       <input type='text' placeholder='Enter Username' name='username'> " +
            "       <label for='pwd'><b>Password</b></label>" +
            "       <input type='password' placeholder='Enter Password' name='password'> " +
            "       <button id='Create'>Create User</button>" +
            "   </form>"

        this.form = this.querySelector("#registerForm")

        this.closeBtn = this.form.querySelector("#registerFormCloseBtn")
        this.closeBtn.addEventListener("click", this.close.bind(this))
    }

    close(){
        (this.parentElement as LoginForm).closeForm();
    }
}

export {RegisterForm}