import {CustomElement, Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {api} from "../RESTApis/RestApi";
import {HHToast} from "hhcommoncomponents";
import {HHForm} from "../Utilities/HHForm";
import {BaseForm} from "../UIComponents/BaseForm";
import {formManager} from "../Utilities/FormManager";
import {RegisterForm} from "./RegisterForm";

// TODO: Extract the framework and make a webflow like lib.
@CustomElement({
    selector: "hh-login-form"
})
class LoginForm extends BaseForm implements HHForm {

    loginBtn: HTMLButtonElement = null;
    registerBtn: HTMLButtonElement = null;
    anonymouseBtn: HTMLButtonElement = null;
    userNameInput: HTMLInputElement
    passwordInput: HTMLInputElement
    afterLogin: Function = null;
    selector: string;

    static get observedAttributes() {
        return ["style"]
    }

   connectedCallback() {
        super.connectedCallback()

        this.modalTitle.innerText = "Login Here"

        let div = document.createElement("div")
        div.innerHTML =
        "       <label for='uname' class='form-label'><b>Username</b></label>" +
        "       <input type='text' class='form-control' placeholder='Enter Username' name='username'> " +
        "       <label for='pwd' class='form-label'><b>Password</b></label>" +
        "       <input type='password' class='form-control' placeholder='Enter Password' name='password'> "

        let footerDiv = document.createElement("div")
        footerDiv.innerHTML =
        "       <button class='btn btn-primary' id='loginBtn'>Login</button>" +
        "       <button class='btn btn-secondary' id='registerBtn'>Register</button>" +
        "       <button class='btn btn-outline-primary' id='anonymousLoginBtn'>Anonymous Login</button>"

        this.footer.appendChild(footerDiv)

        this.form.appendChild(div)

        this.userNameInput = this.querySelector("input[name='username']")
        this.passwordInput = this.querySelector("input[name='password']")

        this.loginBtn = this.querySelector("#loginBtn")
        this.loginBtn.addEventListener("click", this.login.bind(this))

        this.registerBtn = this.querySelector("#registerBtn")
        this.registerBtn.addEventListener("click", this.register.bind(this))
        this.anonymouseBtn = this.querySelector("#anonymousLoginBtn")
        this.anonymouseBtn.addEventListener("click", this.anonymousLogin.bind(this))
    }

    register() {
        formManager.openForm(RegisterForm)
    }

    login(evt){
        return this._login()
    }

    async _login(anonymousLogin: boolean = false) {
        if (userInfo.isLoggedIn) {
            Logger.warn("Has already logged in, no need to login again!")
            return
        }

        if (anonymousLogin) {
            await api.createAnonymousUser()
        } else {
            userInfo.username = this.userNameInput.value
            userInfo.password = this.passwordInput.value
        }

        if (userInfo.username != null && userInfo.password != null && !userInfo.isLoggedIn) {
            let loginResponse = await api.login()

            if (userInfo.isLoggedIn) {
                // HHToast.info("User:" + userInfo.username + " just logged in!")
                HHToast.info(i18n.t("toast.userLoginSuccess", {userName: userInfo.username}))
                this.closeForm() // If logged in successfully, close the form. Or else, leave the form there so user can input username/pwd again.

                // Call back the after login func
                if (this.afterLogin) {
                    this.afterLogin()
                }
            } else{
                let reason = loginResponse?loginResponse.failReason:"Response is empty"
                Logger.error("User:" + userInfo.username + " login failed! Reason:" + reason)
            }
        } else {
            Logger.error("User name or pwd is null!")
        }
    }

    anonymousLogin() {
        this._login(true)
    }
}

export {LoginForm}