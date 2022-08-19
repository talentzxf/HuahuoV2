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