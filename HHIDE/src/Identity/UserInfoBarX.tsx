import * as React from "react"
import {imgButton} from "../UIComponents/MainMenuX";
import {SVGFiles} from "../Utilities/Svgs";
import {formManager} from "../Utilities/FormManager";
import {LoginFormX} from "./LoginFormX";
import {userInfo} from "./UserInfo";

type UserInfoBarState = {
    username: string,
    isLoggedIn: boolean,
}

class UserInfoBarX extends React.Component<any, UserInfoBarState> {
    state: UserInfoBarState = {
        username: i18n.t("not_logged_in"),
        isLoggedIn: false,
    }

    setUserName(val: string) {
        if (!val) {
            this.state.username = i18n.t("not_logged_in")
            this.state.isLoggedIn = false
        } else {
            this.state.username = val
            this.state.isLoggedIn = true
        }

        this.setState(this.state)
    }

    componentDidMount() {
        userInfo.addLoginEventHandler(this.setUserName.bind(this))
    }

    login() {
        formManager.openReactForm(LoginFormX)
    }

    logout() {
        userInfo.logout()
        this.setUserName(null)
    }

    render() {
        let loginButton = imgButton(SVGFiles.signInBtn, i18n.t("Login"), () => {
            this.login()
        })

        let logoutButton = imgButton(SVGFiles.logoutBtn, i18n.t("Logout"), () => {
            this.logout()
        })
        return (<div className="flex flex-row-reverse">
            {this.state.isLoggedIn ? logoutButton : loginButton}
            <span style={this.state.isLoggedIn ? null : {
                cursor: "pointer"
            }} onClick={this.state.isLoggedIn ? null : this.login.bind(this)}>{this.state.username}</span>
        </div>)
    }
}

export {UserInfoBarX}