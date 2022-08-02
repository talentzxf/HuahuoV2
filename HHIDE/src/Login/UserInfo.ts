class UserInfo{
    username: string
    password: string
    isLoggedIn: boolean = false
    jwtToken: string
}

let userInfo = new UserInfo()
export {userInfo}