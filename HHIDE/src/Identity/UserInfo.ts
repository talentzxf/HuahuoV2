class UserInfo{
    username: string
    password: string
    jwtToken: string

    _isLoggedIn: boolean = false

    get isLoggedIn(){
        return this._isLoggedIn
    }

    set isLoggedIn(val:boolean){
        this._isLoggedIn = val
    }
}

let userInfo = new UserInfo()
export {userInfo, UserInfo}