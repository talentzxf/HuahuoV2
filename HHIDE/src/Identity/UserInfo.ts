import {HHToast} from "hhcommoncomponents";

class UserInfo {

    private _isLoggedIn: boolean = false
    private _onLoggedInHandler: Array<Function> = new Array<Function>()

    username: string
    password: string
    jwtToken: string

    get isLoggedIn() {
        return this._isLoggedIn
    }

    addLoginEventHandler(loginEvent: Function) {
        this._onLoggedInHandler.push(loginEvent)
        if(this._isLoggedIn){ // Has already logged in, execute it now.
            loginEvent()
        }
    }

    set isLoggedIn(val: boolean) {
        this._isLoggedIn = val

        if (val) {
            for(let loginEventHandler of this._onLoggedInHandler){
                loginEventHandler(this.username)
            }

            window.localStorage.setItem("username", this.username)
            window.localStorage.setItem("jwtToken", this.jwtToken)
        }
    }

    logout(){
        this._isLoggedIn = false
        window.localStorage.removeItem("username")
        window.localStorage.removeItem("jwtToken")
        window.localStorage.removeItem("password")
        this.username = null
        this.jwtToken = null
        this.password = null

        HHToast.info(i18n.t("toast.userLoggedOut"))
    }
}

let userInfo = new UserInfo()
export {userInfo, UserInfo}