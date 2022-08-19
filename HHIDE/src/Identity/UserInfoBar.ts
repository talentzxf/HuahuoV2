import {CustomElement, Logger} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";
import {api, LoginResponse} from "../RESTApis/RestApi";

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
            this.usernameSpan = document.createElement("span")
            this.appendChild(this.usernameSpan)

            this.username = "Not Logged In"

            userInfo.addLoginEventHandler(this.setUserName.bind(this))

            // Try to login using the saved user info
            let userName = window.localStorage.getItem("username")
            if (userName) {
                let pwd = window.localStorage.getItem("password")
                if (pwd != null) {
                    userInfo.username = userName
                    userInfo.password = pwd
                    userInfo.isLoggedIn = false
                } else {
                    Logger.error("User name is there but pwd is not???")
                }
            }

            api.login().then(()=>{
                Logger.info("User:" + userInfo.username + " logged in!")
            }).catch(()=>{
                Logger.info("User:" + userInfo.username + " login failed!")
            })
        }
    }
}

export {UserInfoBar}