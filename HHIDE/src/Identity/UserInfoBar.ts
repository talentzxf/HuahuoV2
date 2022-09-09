import {CustomElement, Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {api} from "../RESTApis/RestApi";
import {SVGFiles} from "../Utilities/Svgs";

@CustomElement({
    selector: "hh-userinfo-bar"
})
class UserInfoBar extends HTMLElement {
    _username: string
    usernameSpan: HTMLSpanElement

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
            this.appendChild(this.usernameSpan)

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
                            let signOut:HTMLButtonElement = document.createElement("button")
                            signOut.innerHTML = SVGFiles.logoutBtn
                            _this.appendChild(signOut)
                        }
                    })
                } else {
                    Logger.error("User name is there but token is not???")
                }
            }
        }
    }
}

export {UserInfoBar}