import {userInfo} from "./UserInfo";
import {openLoginForm} from "./LoginForm";

function NeedLogin() {
    return function (target: any, propertyKey: string, descriptor: PropertyDescriptor) {

        const realMethod = descriptor.value

        descriptor.value = (...args: any[]) => {
            if (!userInfo.isLoggedIn) {
                openLoginForm(() => {
                    realMethod.apply(target, args)
                })
            } else {
                realMethod.apply(target, args);
            }
        }

        return descriptor;
    }
}

export {NeedLogin}