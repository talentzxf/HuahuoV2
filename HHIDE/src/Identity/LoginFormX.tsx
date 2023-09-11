import * as React from "react"
import {CloseBtn} from "../UIComponents/CloseBtn";
import {userInfo} from "./UserInfo";
import {HHToast, Logger} from "hhcommoncomponents";
import {api} from "../RESTApis/RestApi";
import {formManager, FormProps} from "../Utilities/FormManager";
import {RegisterForm} from "./RegisterForm";
import {AxiosError} from "axios";
import {CSSUtils} from "../Utilities/CSSUtils";

type LoginState = {
    username: string
    password: string
    isVisible: boolean
}

type LoginProps = FormProps & {
    afterLogin: Function
    onError: Function
    onLoginFailed: Function
}

function getBtnClz(color: string) {
    let btnClz = CSSUtils.getButtonClass(color)
    btnClz += " p-3 m-1 rounded-lg"
    return btnClz
}

class LoginFormX extends React.Component<LoginProps, LoginState> {
    state: LoginState = {
        username: "unknown",
        password: "unknown",
        isVisible: true,
    }

    async _login(anonymousLogin: boolean = false) {
        if (userInfo.isLoggedIn) {
            Logger.warn("Has already logged in, no need to login again!")
            return
        }

        if (anonymousLogin) {
            await api.createAnonymousUser()
        } else {
            userInfo.username = this.state.username
            userInfo.password = this.state.password
        }

        if (userInfo.username != null && userInfo.password != null && !userInfo.isLoggedIn) {
            // @ts-ignore
            try {


                let loginResponse = await api.login()

                if (userInfo.isLoggedIn) {
                    HHToast.info(i18n.t("toast.userLoginSuccess", {userName: userInfo.username}))
                    this.props?.closeForm() // If logged in successfully, close the form. Or else, leave the form there so user can input username/pwd again.

                    // Call back the after login func
                    if (this.props.afterLogin) {
                        this.props.afterLogin()
                    }
                } else {
                    let reason = loginResponse ? loginResponse.failReason : "Response is empty"
                    let errorString = "User:" + userInfo.username + " login failed! Reason:" + reason

                    if (this.props.onLoginFailed) {
                        this.props.onLoginFailed(errorString)
                    }
                }
            } catch (error: any) {
                let e = error as AxiosError
                if (this.props.onError) {
                    this.props.onError(e.message)
                }
            }
        } else {
            Logger.error("User name or pwd is null!")
        }
    }

    onLogin(e: Event) {
        this._login()

        e.preventDefault()
    }

    onAnonymousLogin(e: Event) {
        this._login(true)
        e.preventDefault()
    }

    onRegister(e: Event) {
        formManager.openForm(RegisterForm)
        e.preventDefault()
    }

    onUserNameChanged(e) {
        this.state.username = e.target.value
    }

    onPasswordChanged(e) {
        this.state.password = e.target.value
    }

    componentDidMount() {
        console.log("Component is mounted!")
    }

    render() {
        return (
            <div className="flex flex-col items-center justify-center mx-auto">
                <div
                    className="w-full bg-white rounded-lg drop-shadow-2xl dark:border md:mt-0 sm:max-w-md xl:p-0 dark:bg-gray-800 dark:border-gray-700">
                    <form className="p-4 space-y-4 divide-y divide-gray-300" action="#">
                        <div className="flex align-middle">
                            <h5 className="px-2 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                                {i18n.t("Login")}
                            </h5>
                            <CloseBtn onclick={this.props?.closeForm}></CloseBtn>
                        </div>
                        <div>
                            <div>
                                <label
                                    htmlFor="username"
                                    className="block mb-2 text-sm font-medium text-gray-900 dark:text-white"
                                >
                                    <b>Username</b>
                                </label>
                                <input
                                    name="username"
                                    id="username"
                                    className="bg-gray-50 border border-gray-300 text-gray-900 sm:text-sm rounded-lg focus:ring-primary-600 focus:border-primary-600 block w-full p-2.5 dark:bg-gray-700 dark:border-gray-600 dark:placeholder-gray-400 dark:text-white dark:focus:ring-blue-500 dark:focus:border-blue-500"
                                    placeholder="username"
                                    onChange={this.onUserNameChanged.bind(this)}
                                />
                            </div>
                            <div>
                                <label
                                    htmlFor="password"
                                    className="block mb-2 text-sm font-medium text-gray-900 dark:text-white"
                                >
                                    <b>Password</b>
                                </label>
                                <input
                                    type="password"
                                    name="password"
                                    id="password"
                                    placeholder="••••••••"
                                    className="bg-gray-50 border border-gray-300 text-gray-900 sm:text-sm rounded-lg focus:ring-primary-600 focus:border-primary-600 block w-full p-2.5 dark:bg-gray-700 dark:border-gray-600 dark:placeholder-gray-400 dark:text-white dark:focus:ring-blue-500 dark:focus:border-blue-500"
                                    onChange={this.onPasswordChanged.bind(this)}
                                />
                            </div>
                        </div>

                        <div className="p-2 flex flex-row">
                            <button
                                className={getBtnClz("primary")}
                                onClick={this.onLogin.bind(this)}
                            >
                                Login
                            </button>
                            <button
                                className={getBtnClz("emerald")}
                                onClick={this.onRegister.bind(this)}
                            >
                                Register
                            </button>
                            <button
                                className={getBtnClz("fuchsia")}
                                onClick={this.onAnonymousLogin.bind(this)}
                            >
                                Anonymous Login
                            </button>
                        </div>
                    </form>
                </div>
            </div>
        )
    }
}

export {LoginFormX}