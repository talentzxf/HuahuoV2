import {Logger} from "hhcommoncomponents"
import axios from "axios";
import {userInfo} from "../Login/UserInfo";
import huahuoProperties from "../hhide.properties";

class CreateUserResponse {
    userName: string
    password: string
}

class RestApi{
    baseUrl: string;

    constructor() {
        this.baseUrl = huahuoProperties["huahuo.backend.url"]
    }

    async createAnonymousUser() {
        let loginUrl = this.baseUrl + "/users"

        try {
            const {data, status} = await axios.post<CreateUserResponse>(
                loginUrl,
                null,
                {
                    headers: {
                        isAnonymous: true,
                        'Content-Type': 'application/json'
                    }
                }
            )

            console.log(data)
            window.localStorage.setItem("username", data.userName)
            window.localStorage.setItem("password", data.password)

            userInfo.username = data.userName
            userInfo.password = data.password
            userInfo.isLoggedIn = false
        } catch (error) {
            if (axios.isAxiosError(error)) {
                Logger.error("Axios error happened!", error.message);
                return error.message
            } else {
                Logger.error("Unexpected error happened!", error);
                return "Unexcepted error happened!"
            }
        }
    }
}

let api = new RestApi()
export {api}