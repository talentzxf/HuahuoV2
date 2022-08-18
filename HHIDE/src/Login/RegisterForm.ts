import {CustomElement} from "hhcommoncomponents"
import {LoginForm} from "./LoginForm";
import {api} from "../RESTApis/RestApi";

@CustomElement({
    selector: "hh-register-form"
})
class RegisterForm extends HTMLElement {
    closeBtn: HTMLButtonElement
    form: HTMLFormElement
    userNameInput: HTMLInputElement
    createUserBtn: HTMLButtonElement

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
            "       <input type='text' placeholder='Enter Username' id='username'> " +
            "       <label for='pwd'><b>Password</b></label>" +
            "       <input type='password' placeholder='Enter Password' id='password'> " +
            "       <input type='password' placeholder='Retype Password' id='retype_password'> " +
            "       <button id='Create' name='create_user_btn'>Create User</button>" +
            "   </form>"

        this.form = this.querySelector("#registerForm")

        this.form.addEventListener("submit", function (e) {
            e.stopPropagation()
            e.preventDefault()
        })

        this.userNameInput = this.form.querySelector("#username")
        this.closeBtn = this.form.querySelector("#registerFormCloseBtn")
        this.closeBtn.addEventListener("click", this.close.bind(this))

        this.createUserBtn = this.form.querySelector("#create_user_btn")

        this.userNameInput.addEventListener("input", this.onUserNameChanged.bind(this))
    }

    onUserNameChanged(e:KeyboardEvent){
        let curUserName = this.userNameInput.value

        api.isUserExist(curUserName, this.userExistCallback.bind(this), this.userNotExistCallback.bind(this))
    }

    userExistCallback(){

    }

    userNotExistCallback(){

    }

    close(){
        (this.parentElement as LoginForm).closeForm();
    }
}

export {RegisterForm}