import {CustomElement, Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {api, LoginResponse} from "../RESTApis/RestApi";
import {HHToast} from "hhcommoncomponents";
import {CSSDefines} from "../Utilities/CSSDefines";

let loginForm = null;

const css2obj = css => {

    if(!css)
        return {}

    const r = /(?<=^|;)\s*([^:]+)\s*:\s*([^;]+)\s*/g, o = {};
    css.replace(r, (m,p,v) => o[p] = v);
    return o;

}

// TODO: Extract the framework and make a webflow like lib.
@CustomElement({
    selector: "hh-login-form"
})
class LoginForm extends HTMLElement {
    closeBtn: HTMLElement = null;
    loginForm: HTMLFormElement = null;
    registerForm: HTMLElement = null;

    loginBtn: HTMLButtonElement = null;
    registerBtn: HTMLButtonElement = null;
    anonymouseBtn: HTMLButtonElement = null;
    loginFormContainer: HTMLElement = null;
    userNameInput: HTMLInputElement
    passwordInput: HTMLInputElement
    afterLogin: Function = null;

    static get observedAttributes(){
        return ["style"]
    }

    attributeChangedCallback(name, oldValue, newValue){
        if(name == "style"){
            let oldCssObj:object = css2obj(oldValue)
            let newCssObj:object = css2obj(newValue)

            if(oldCssObj["display"] == "none" && newCssObj["display"] != "none"){

                if(this.registerForm)
                    this.registerForm.style.display = "none"
                if(this.loginForm)
                    this.loginForm.style.display = "block"
            }
        }
    }


    connectedCallback() {
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"
        this.loginFormContainer = document.createElement("div")
        this.loginFormContainer.innerHTML = CSSDefines.formStyle

        this.loginFormContainer.innerHTML +=
            "   <form id='loginForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='loginFormCloseBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "       <h3>Login Here</h3>" +
            "       <label for='uname'><b>Username</b></label>" +
            "       <input type='text' placeholder='Enter Username' name='username'> " +
            "       <label for='pwd'><b>Password</b></label>" +
            "       <input type='password' placeholder='Enter Password' name='password'> " +
            "       <button id='loginBtn'>Login</button>" +
            "       <button id='registerBtn'>Register</button>" +
            "       <button id='anonymousLoginBtn'>Anonymous Login</button>" +
            "   </form>"

        this.appendChild(this.loginFormContainer)

        this.loginForm = this.loginFormContainer.querySelector("form")
        this.loginForm.addEventListener("submit", function (e) {
            e.stopPropagation()
            e.preventDefault()
        })

        this.userNameInput = this.querySelector("input[name='username']")
        this.passwordInput = this.querySelector("input[name='password']")

        this.closeBtn = this.loginFormContainer.querySelector("#loginFormCloseBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.loginBtn = this.loginFormContainer.querySelector("#loginBtn")
        this.loginBtn.addEventListener("click", this.login.bind(this))

        this.registerBtn = this.loginFormContainer.querySelector("#registerBtn")
        this.registerBtn.addEventListener("click", this.register.bind(this))
        this.anonymouseBtn = this.loginFormContainer.querySelector("#anonymousLoginBtn")
        this.anonymouseBtn.addEventListener("click", this.anonymousLogin.bind(this))
    }

    register(){
        this.loginForm.style.display = "none"

        if(!this.registerForm){
            this.registerForm = document.createElement("hh-register-form")
            this.appendChild(this.registerForm)
        }

        this.registerForm.style.display = "block"
    }

    login() {
        try {
            this._login();
        } finally {
            this.closeForm();
        }
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
            let loginResponse: LoginResponse = await api.login()

            if (userInfo.isLoggedIn) {
                // HHToast.info("User:" + userInfo.username + " just logged in!")
                HHToast.info(i18n.t("toast.userLoginSuccess", {userName: userInfo.username}))

                // Call back the after login func
                if (this.afterLogin) {
                    if (userInfo.isLoggedIn) {
                        this.afterLogin()
                    }
                }
            } else
                Logger.error("User:" + userInfo.username + " login failed! Reason:" + loginResponse.failReason)
        } else {
            Logger.error("User name or pwd is null!")
        }
    }

    anonymousLogin() {
        try {
            this._login(true)
        } finally {
            this.closeForm()
        }
    }

    closeForm() {
        this.style.display = "none";
    }
}

function openLoginForm(afterLoginAction: Function = null) {

    let needAppend = false
    if (loginForm == null) {
        loginForm = document.createElement("hh-login-form")
        needAppend = true
    }

    loginForm.style.display = "block"
    loginForm.afterLogin = afterLoginAction

    if(needAppend){
        document.body.appendChild(loginForm)
    }
}

export {openLoginForm, LoginForm}