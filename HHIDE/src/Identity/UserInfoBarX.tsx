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
        this.state.username = val
        if(val){
            this
        }
    }

    componentDidMount() {
        userInfo.addLoginEventHandler(this.setUserName.bind(this))
    }

    render() {
        return (<>
            <span></span>
            {imgButton(SVGFiles.signInBtn, i18n.t("Login"), () => {
                formManager.openReactForm(LoginFormX)
            })}
        </>)
    }
}