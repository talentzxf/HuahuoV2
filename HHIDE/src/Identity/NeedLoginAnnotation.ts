import {userInfo} from "./UserInfo";
import {formManager} from "../Utilities/FormManager";
import {LoginForm} from "./LoginForm";

function NeedLogin() {
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor) {

        const realMethod = descriptor.value

        descriptor.value = (...args: any[]) => {
            if (!userInfo.isLoggedIn) {
                let loginForm = formManager.openForm(LoginForm)

                let resolveFunction:Function
                let promise = new Promise((resolve, reject)=>{
                    resolveFunction = resolve
                })

                loginForm.afterLogin = () => {
                        let realMethodReturn = realMethod.apply(target, args)

                        if(realMethodReturn.constructor.name === "Promise")
                        {
                            realMethodReturn.then((response)=>{
                                resolveFunction(response)
                            })

                        }

                        return realMethodReturn
                    }

                return promise
            } else {
                return realMethod.apply(target, args);
            }
        }

        return descriptor;
    }
}

export {NeedLogin}