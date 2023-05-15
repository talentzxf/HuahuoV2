import {CustomElement, Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {api} from "../RESTApis/RestApi";
import {SVGFiles} from "../Utilities/Svgs";
import {formManager} from "../Utilities/FormManager";
import {LoginForm} from "./LoginForm";

@CustomElement({
    selector: "hh-userinfo-bar"
})
class UserInfoBar extends HTMLElement {
    _username: string
    usernameSpan: HTMLSpanElement
    loginLogoutBtn: HTMLButtonElement

    get username() {
        return this._username
    }

    set username(val: string) {
        this._username = val

        if (!val) {
            this.usernameSpan.innerText = i18n.t("not_logged_in")
            this.usernameSpan.style.cursor = 'pointer'
            this.usernameSpan.onclick = this.login.bind(this)
        } else {
            this.usernameSpan.innerText = this._username
        }
    }

    setUserName(val: string) {
        this.username = val

        if (val) {
            // Change to log out button
            this.loginLogoutBtn.innerHTML = SVGFiles.logoutBtn
            this.loginLogoutBtn.onclick = this.logout.bind(this)
        } else {
            // Change to sign in button
            this.loginLogoutBtn.innerHTML = SVGFiles.signInBtn
            this.loginLogoutBtn.onclick = this.login.bind(this)
        }
    }

    connectedCallback() {
        if (!this.usernameSpan) {
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
            this.username = null

            userInfo.addLoginEventHandler(this.setUserName.bind(this))

            // Try to validate if the jwtToken is still valid
            let userName = window.localStorage.getItem("username")
            if (userName) {
                let token = window.localStorage.getItem("jwtToken")
                if (token != null) {
                    api.isTokenValid(userName, token).then((response: any) => {
                        if (response && response["isValid"]) {
                            userInfo.username = userName
                            userInfo.jwtToken = token
                            userInfo.isLoggedIn = true

                            _this.setUserName(userName)
                        }
                    })
                } else {
                    Logger.error("User name is there but token is not???")
                }
            }
        }
    }

    login() {
        formManager.openForm(LoginForm)
    }

    logout() {
        userInfo.logout()
        this.setUserName(null)
    }
}

export {UserInfoBar}