import {CustomElement, Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {api} from "../RESTApis/RestApi";
import {SVGFiles} from "../Utilities/Svgs";
import {openLoginForm} from "./LoginForm";

@CustomElement({
    selector: "hh-userinfo-bar"
})
class UserInfoBar extends HTMLElement {
    _username: string
    usernameSpan: HTMLSpanElement
    loginLogoutBtn: HTMLButtonElement

    get username(){
        return this._username
    }

    set username(val: string){
        this._username = val
        this.usernameSpan.innerText = this._username
    }

    setUserName(val: string){
        this.username = val
    }

    connectedCallback(){
        if(!this.usernameSpan){
            this.style.display = "flex"
            this.usernameSpan = document.createElement("span")
            this.usernameSpan.style.lineHeight = "30px"
            this.usernameSpan.style.verticalAlign = "middle"

            this.loginLogoutBtn = document.createElement("button")
            this.loginLogoutBtn.innerHTML = SVGFiles.signInBtn
            this.appendChild(this.usernameSpan)
            this.appendChild(this.loginLogoutBtn)

            this.loginLogoutBtn.onclick = this.login

            let _this = this
            let i18n = (window as any).i18n
            this.username = i18n.t("not_logged_in")

            userInfo.addLoginEventHandler(this.setUserName.bind(this))

            // Try to validate if the jwtToken is still valid
            let userName = window.localStorage.getItem("username")
            if (userName) {
                let token = window.localStorage.getItem("jwtToken")
                if (token != null) {
                    api.isTokenValid(userName, token).then((response: any)=>{
                        if(response && response["isValid"]){
                            userInfo.username = userName
                            userInfo.jwtToken = token
                            userInfo.isLoggedIn = true

                            _this.setUserName(userName)

                            // Add a log out button
                            _this.loginLogoutBtn.innerHTML = SVGFiles.logoutBtn
                            _this.loginLogoutBtn.onclick = _this.logout.bind(_this)
                        }
                    })
                } else {
                    Logger.error("User name is there but token is not???")
                }
            }
        }
    }

    login(){
        openLoginForm()
    }

    logout(){

    }
}

export {UserInfoBar}