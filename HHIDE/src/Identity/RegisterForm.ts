import {CustomElement} from "hhcommoncomponents"
import {LoginForm} from "./LoginForm";
import {api} from "../RESTApis/RestApi";
import {Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {SVGFiles} from "../Utilities/Svgs";
import {HHForm} from "../Utilities/HHForm";
import * as string_decoder from "string_decoder";
import {BaseForm} from "../UIComponents/BaseForm";

@CustomElement({
    selector: "hh-register-form"
})
class RegisterForm extends BaseForm{
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
        super.connectedCallback()
        let formDiv = document.createElement("div")

        this.modalTitle.innerText = "Register Here"

        formDiv.innerHTML =
            "       <label for='uname' class='form-label'><b>UserName</b></label>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input class='form-control' type='text' placeholder='Enter Username' id='username'> " +
            "           <img id='userNameCheckImg' style='width:20px; height:20px'> " +
            "       </div>" +
            "       <label class='form-label' for='nicknameLabel'><b>NickName</b></label>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input class='form-control' type='text' placeholder='Enter NickName' id='nickname'> " +
            "           <span style='width:20px; height:20px'> " +
            "       </div>" +
            "       <label class='form-label' for='pwd'><b>Password</b></label>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input class='form-control' type='password' placeholder='Enter Password' id='password'> " +
            "           <img id='passwordCheckImg' style='width:20px; height:20px'> " +
            "       </div>" +
            "       <div style='display: flex; align-items: center'>" +
            "           <input class='form-control' type='password' placeholder='ReEnter Password' id='password_retype'> " +
            "           <span style='width:20px; height:20px'> " +
            "       </div>"

        this.form.appendChild(formDiv)

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

        this.createUserBtn = document.createElement("button")
        this.createUserBtn.className = "btn btn-primary disabled"
        this.createUserBtn.innerText = "Create User"
        this.footer.appendChild(this.createUserBtn)
        this.createUserBtn.addEventListener("click", this.onCreateUser.bind(this))
    }

    checkAndSetSubmitButton(){
        if(this.isAbleToSubmit){
            this.createUserBtn.classList.remove("disabled")
        }else {
            this.createUserBtn.classList.add("disabled")
        }
    }

    onNickNameKeyDown(e:KeyboardEvent){
        this.nickNameSyncFlag = false

        this.checkAndSetSubmitButton()
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

        this.checkAndSetSubmitButton()
    }

    userNotExistCallback(){
        this.isUserNameValid = true

        this.checkAndSetSubmitButton()
    }

    passwordInputChanged(){
        let rawPwd = this.passwordInput.value;
        let retypedPwd = this.passwordRetypeInput.value;

        if(rawPwd && rawPwd.length > 0 && rawPwd === retypedPwd){
            this.isPasswordValid = true
        }else{
            this.isPasswordValid = false
        }

        this.checkAndSetSubmitButton()
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
                _this.closeForm()
            })
        }
    }

    closeForm(){
        this.style.display = "none"
    }
}

export {RegisterForm}