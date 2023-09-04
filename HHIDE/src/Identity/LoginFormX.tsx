import * as React from "react"
import {HHReactForm} from "../Utilities/HHForm";
import {CloseBtn} from "../UIComponents/CloseBtn";

type LoginState = {
    username: string
    password: string
    style: {
        display: string
    }
}

class LoginFormX extends React.Component<any, LoginState> implements HHReactForm {
    state: LoginState = {
        username: "unknown",
        password: "unknown",
        style: {
            display: "block"
        }
    }

    getButtonClass(color: string) {
        let btnClass: string = `m-1 p-3 text-white bg-${color}-600 hover:bg-${color}-700 focus:ring-4 focus:outline-none focus:ring-${color}-300 ` +
            `font-medium rounded-lg text-sm text-center dark:bg-${color}-600 dark:hover:bg-${color}-700 dark:focus:ring-${color}-800`
        return btnClass
    }

    onSubmit(e: Event) {
        console.log(this.state.username)

        e.preventDefault()
    }

    onUserNameChanged(e) {
        this.state.username = e.target.value
    }

    onClose() {
        this.state.style = {
            display: "none"
        }

        this.setState(this.state)
    }

    componentDidMount() {
        console.log("Component is mounted!")
    }

    render() {
        return (
            <div className="flex flex-col items-center justify-center mx-auto" style={this.state.style}>
                <div
                    className="w-full bg-white rounded-lg drop-shadow-2xl dark:border md:mt-0 sm:max-w-md xl:p-0 dark:bg-gray-800 dark:border-gray-700">
                    <form className="p-4 space-y-4 divide-y divide-gray-300" action="#">
                        <div className="flex align-middle">
                            <h5 className="p-4 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                                Login
                            </h5>
                            <div className="w-full"></div>
                            <CloseBtn onclick={this.onClose.bind(this)}></CloseBtn>
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
                                />
                            </div>
                        </div>

                        <div className="p-2 flex flex-row">
                            <button
                                className={this.getButtonClass("primary")}
                                onClick={this.onSubmit.bind(this)}
                            >
                                Login
                            </button>
                            <button
                                className={this.getButtonClass("emerald")}
                                onClick={this.onSubmit.bind(this)}
                            >
                                Register
                            </button>
                            <button
                                className={this.getButtonClass("fuchsia")}
                                onClick={this.onSubmit.bind(this)}
                            >
                                Anonymous Login
                            </button>
                        </div>
                    </form>
                </div>
            </div>
        )
    }

    closeForm() {
    }
}

export {LoginFormX}