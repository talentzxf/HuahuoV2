import {CustomElement} from "hhcommoncomponents"
import {LoginForm} from "./LoginForm";
import {api} from "../RESTApis/RestApi";
import {Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";

@CustomElement({
    selector: "hh-register-form"
})
class RegisterForm extends HTMLElement {
    closeBtn: HTMLButtonElement
    form: HTMLFormElement
    userNameInput: HTMLInputElement
    passwordInput: HTMLInputElement
    passwordRetypeInput: HTMLInputElement
    createUserBtn: HTMLButtonElement
    nickNameInput: HTMLInputElement

    nickNameSyncFlag: Boolean = true

    _isUserNameValid: Boolean = false
    _isPasswordValid: Boolean = false

    userNameCheckImg: HTMLImageElement;
    passwordCheckImg: HTMLImageElement;
    okImg: string = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' width='256' height='256' viewBox='0 0 256 256' xml:space='preserve'%3E%3Cdefs%3E%3C/defs%3E%3Cg style='stroke: none%3B stroke-width: 0%3B stroke-dasharray: none%3B stroke-linecap: butt%3B stroke-linejoin: miter%3B stroke-miterlimit: 10%3B fill: none%3B fill-rule: nonzero%3B opacity: 1%3B' transform='translate(1.4065934065934016 1.4065934065934016) scale(2.81 2.81)' %3E%3Ccircle cx='45' cy='45' r='45' style='stroke: none%3B stroke-width: 1%3B stroke-dasharray: none%3B stroke-linecap: butt%3B stroke-linejoin: miter%3B stroke-miterlimit: 10%3B fill: rgb(93 201 121)%3B fill-rule: nonzero%3B opacity: 1%3B' transform=' matrix(1 0 0 1 0 0) '/%3E%3Cpath d='M 69.643 24.047 c -0.566 -0.72 -1.611 -0.862 -2.342 -0.309 c -6.579 4.976 -12.729 10.529 -18.402 16.574 c -3.022 3.229 -5.905 6.603 -8.599 10.161 c -1.431 1.904 -2.808 3.86 -4.109 5.889 h -0.082 l -7.6 -15.206 c -0.073 -0.146 -0.157 -0.293 -0.247 -0.433 c -1.401 -2.168 -4.343 -2.741 -6.462 -1.209 c -1.987 1.437 -2.337 4.279 -1.006 6.339 l 12.304 19.043 l 0.151 0.234 c 0.435 0.676 1.13 1.198 2.019 1.398 c 1.525 0.343 3.04 -0.567 3.671 -1.997 c 1.623 -3.678 3.724 -7.281 6.016 -10.76 c 2.337 -3.525 4.898 -6.936 7.614 -10.228 c 5.102 -6.169 10.74 -11.942 16.842 -17.188 C 70.093 25.768 70.199 24.754 69.643 24.047 z' style='stroke: none%3B stroke-width: 1%3B stroke-dasharray: none%3B stroke-linecap: butt%3B stroke-linejoin: miter%3B stroke-miterlimit: 10%3B fill: rgb(255 255 255)%3B fill-rule: nonzero%3B opacity: 1%3B' transform=' matrix(1 0 0 1 0 0) ' stroke-linecap='round' /%3E%3C/g%3E%3C/svg%3E"
    notOkImg: string = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' width='256' height='256' viewBox='0 0 256 256' xml:space='preserve'%3E%3Cdefs%3E%3C/defs%3E%3Cg style='stroke: none%3B stroke-width: 0%3B stroke-dasharray: none%3B stroke-linecap: butt%3B stroke-linejoin: miter%3B stroke-miterlimit: 10%3B fill: none%3B fill-rule: nonzero%3B opacity: 1%3B' transform='translate(1.4065934065934016 1.4065934065934016) scale(2.81 2.81)' %3E%3Cpath d='M 45 89.979 c -11.523 0 -23.047 -4.387 -31.82 -13.159 c -17.545 -17.546 -17.545 -46.095 0 -63.64 c 17.546 -17.546 46.093 -17.546 63.64 0 c 17.545 17.545 17.545 46.094 0 63.64 C 68.047 85.593 56.523 89.979 45 89.979 z M 45 14.015 c -7.938 0 -15.877 3.021 -21.92 9.065 c -12.087 12.087 -12.087 31.753 0 43.84 c 12.087 12.088 31.754 12.088 43.84 0 c 12.087 -12.087 12.087 -31.753 0 -43.84 C 60.877 17.036 52.938 14.015 45 14.015 z' style='stroke: none%3B stroke-width: 1%3B stroke-dasharray: none%3B stroke-linecap: butt%3B stroke-linejoin: miter%3B stroke-miterlimit: 10%3B fill: rgb(255 64 64)%3B fill-rule: nonzero%3B opacity: 1%3B' transform=' matrix(1 0 0 1 0 0) ' stroke-linecap='round' /%3E%3Crect x='7' y='38' rx='0' ry='0' width='76' height='14' style='stroke: none%3B stroke-width: 1%3B stroke-dasharray: none%3B stroke-linecap: butt%3B stroke-linejoin: miter%3B stroke-miterlimit: 10%3B fill: rgb(255 64 64)%3B fill-rule: nonzero%3B opacity: 1%3B' transform=' matrix(0.7071 -0.7071 0.7071 0.7071 -18.6396 44.9969) '/%3E%3C/g%3E%3C/svg%3E"

    get isAbleToSubmit(){
        return this._isUserNameValid && this._isPasswordValid
    }

    set isUserNameValid(val:Boolean){
        this._isUserNameValid = val;

        if(val)
            this.userNameCheckImg.src = this.okImg
        else
            this.userNameCheckImg.src = this.notOkImg
    }

    set isPasswordValid(val:Boolean){
        this._isPasswordValid = val;

        if(val)
            this.passwordCheckImg.src = this.okImg
        else
            this.passwordCheckImg.src = this.notOkImg
    }

    connectedCallback(){
        this.innerHTML =
            "   <form id='registerForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='registerFormCloseBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "       <h3>Register Here</h3>" +
            "       <label for='uname'><b>UserName</b></label>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input type='text' placeholder='Enter Username' id='username'> " +
            "           <img id='userNameCheckImg' style='width:20px; height:20px'> " +
            "       </div>" +
            "       <label for='nicknameLabel'><b>NickName</b></label>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input type='text' placeholder='Enter NickName' id='nickname'> " +
            "           <span style='width:20px; height:20px'> " +
            "       </div>" +
            "       <label for='pwd'><b>Password</b></label>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input type='password' placeholder='Enter Password' id='password'> " +
            "           <img id='passwordCheckImg' style='width:20px; height:20px'> " +
            "       </div>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input type='password' placeholder='ReEnter Password' id='password_retype'> " +
            "           <span style='width:20px; height:20px'> " +
            "       </div>" +
            "       <button id='createUserBtn'>Create User</button>" +
            "   </form>"

        this.form = this.querySelector("#registerForm")

        this.form.addEventListener("submit", function (e) {
            e.stopPropagation()
            e.preventDefault()
        })

        this.userNameCheckImg = this.querySelector("#userNameCheckImg")
        this.userNameCheckImg.src = this.notOkImg

        this.userNameInput = this.form.querySelector("#username")
        this.userNameInput.addEventListener("input", this.onUserNameChanged.bind(this))

        this.nickNameInput = this.form.querySelector("#nickname")
        this.nickNameInput.addEventListener("keydown", this.onNickNameKeyDown.bind(this))

        this.passwordInput = this.form.querySelector("#password")
        this.passwordRetypeInput = this.form.querySelector("#password_retype")

        this.passwordInput.addEventListener("input", this.passwordInputChanged.bind(this))
        this.passwordRetypeInput.addEventListener("input", this.passwordInputChanged.bind(this))

        this.passwordCheckImg = this.form.querySelector("#passwordCheckImg")
        this.passwordCheckImg.src = this.notOkImg

        this.closeBtn = this.form.querySelector("#registerFormCloseBtn")
        this.closeBtn.addEventListener("click", this.close.bind(this))

        this.createUserBtn = this.form.querySelector("#createUserBtn")
        this.createUserBtn.addEventListener("click", this.onCreateUser.bind(this))
    }

    onNickNameKeyDown(e:KeyboardEvent){
        this.nickNameSyncFlag = false
    }

    onUserNameChanged(e:KeyboardEvent){
        let curUserName = this.userNameInput.value

        if(this.nickNameSyncFlag){
            this.nickNameInput.value = curUserName
        }

        if(curUserName && curUserName.length > 0)
            api.isUserExist(curUserName, this.userExistCallback.bind(this), this.userNotExistCallback.bind(this))
        else
            this.userNotExistCallback()
    }

    userExistCallback(){
        this.isUserNameValid = false
    }

    userNotExistCallback(){
        this.isUserNameValid = true
    }

    passwordInputChanged(){
        let rawPwd = this.passwordInput.value;
        let retypedPwd = this.passwordRetypeInput.value;

        if(rawPwd && rawPwd.length > 0 && rawPwd === retypedPwd){
            this.isPasswordValid = true
        }else{
            this.isPasswordValid = false
        }
    }

    onCreateUser(){
        if(!this.isAbleToSubmit){
            Logger.error("Username/pwd not correct!")
        }else{
            let username = this.userNameInput.value
            let password = this.passwordInput.value
            let nickname = this.nickNameInput.value

            api.createUser(username, password, nickname).then(()=>{
                userInfo.username = username
                userInfo.password = password
                api.login()
            })
        }
    }

    close(){
        (this.parentElement as LoginForm).closeForm();
    }
}

export {RegisterForm}