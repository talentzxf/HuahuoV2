import {CustomElement} from "hhcommoncomponents"
import {LoginForm} from "./LoginForm";
import {api} from "../RESTApis/RestApi";
import {Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {SVGFiles} from "../Utilities/Svgs";

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
    okImg: string = SVGFiles.okImg
    notOkImg: string = SVGFiles.notOKImg

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

            let _this = this

            api.createUser(username, password, nickname).then((response)=>{
                userInfo.username = username
                userInfo.password = password
                api.login()
                _this.close()
            })
        }
    }

    close(){
        (this.parentElement as LoginForm).closeForm();
    }
}

export {RegisterForm}