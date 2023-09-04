import {userInfo} from "./UserInfo";
import {formManager} from "../Utilities/FormManager";
import {LoginFormX} from "./LoginFormX";
import {HHToast} from "hhcommoncomponents";

function NeedLogin() {
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor) {

        const realMethod = descriptor.value

        descriptor.value = (...args: any[]) => {
            if (!userInfo.isLoggedIn) {
                let resolveFunction: Function
                let promise = new Promise((resolve, reject) => {
                    resolveFunction = resolve
                })

                let afterLoginFunc = () => {
                    let realMethodReturn = realMethod.apply(target, args)

                    if (realMethodReturn && realMethodReturn.constructor.name === "Promise") {
                        realMethodReturn.then((response) => {
                            resolveFunction(response)
                        })

                    }

                    return realMethodReturn
                }

                formManager.openReactForm(LoginFormX, {
                    afterLogin: afterLoginFunc,
                    onError: (errMsg) => {
                        HHToast.error(errMsg)
                    },
                    onLoginFailed: (errMsg) => {
                        HHToast.warn(errMsg)
                    }
                })

                return promise
            } else {
                return realMethod.apply(target, args);
            }
        }

        return descriptor;
    }
}

export {NeedLogin}