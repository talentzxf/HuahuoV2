import * as React from "react"
import {useState} from "react"
import {HHReactForm} from "../Utilities/HHForm";

type LoginProps = {}

type LoginState = {
    username: string
    password: string
}

class LoginFormX extends React.Component<LoginProps, LoginState> implements HHReactForm {
    state: LoginState = {
        username: "unknown",
        password: "unknown"
    }

    props: LoginProps = {}

    onSubmit(e: Event) {
        console.log(this.state.username)

        e.preventDefault()
    }

    onUserNameChanged(e) {
        this.state.username = e.target.value
    }

    render() {
        return (
            <div className="flex flex-col items-center justify-center mx-auto">
                <div
                    className="w-full bg-white rounded-lg shadow dark:border md:mt-0 sm:max-w-md xl:p-0 dark:bg-gray-800 dark:border-gray-700">
                    <form className="p-4 space-y-4 divide-y divide-gray-300" action="#">
                        <h5 className="p-4 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                            Login Here
                        </h5>
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
                                type="submit"
                                className="w-full p-1 text-white bg-primary-600 hover:bg-primary-700 focus:ring-4 focus:outline-none focus:ring-primary-300
                                    font-medium rounded-lg text-sm px-5 py-2.5 text-center dark:bg-primary-600 dark:hover:bg-primary-700 dark:focus:ring-primary-800"
                                onClick={this.onSubmit.bind(this)}
                            >
                                Sign in
                            </button>
                            <button
                                type="submit"
                                className="w-full p-1 text-white bg-primary-600 hover:bg-primary-700 focus:ring-4 focus:outline-none focus:ring-primary-300
                                    font-medium rounded-lg text-sm px-5 py-2.5 text-center dark:bg-primary-600 dark:hover:bg-primary-700 dark:focus:ring-primary-800"
                                onClick={this.onSubmit.bind(this)}
                            >
                                Register
                            </button>
                            <button
                                type="submit"
                                className="w-full p-1 text-white bg-primary-600 hover:bg-primary-700 focus:ring-4 focus:outline-none focus:ring-primary-300
                                    font-medium rounded-lg text-sm px-5 py-2.5 text-center dark:bg-primary-600 dark:hover:bg-primary-700 dark:focus:ring-primary-800"
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