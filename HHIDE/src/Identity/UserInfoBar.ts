import {CustomElement} from "hhcommoncomponents";
import {userInfo} from "./UserInfo";

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
        }
    }
}

export {UserInfoBar}