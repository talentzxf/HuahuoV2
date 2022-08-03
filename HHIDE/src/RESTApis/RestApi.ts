import {Logger} from "hhcommoncomponents"
import axios from "axios";
import {userInfo} from "../Login/UserInfo";
import huahuoProperties from "../hhide.properties";

class CreateUserResponse {
    username: string
    password: string
}

class LoginResponse {
    userName: string
    failReason: null
    httpStatus: string
}

class RestApi {
    baseUrl: string;

    constructor() {
        this.baseUrl = huahuoProperties["huahuo.backend.url"]
    }

    async _callApi<T>(url: string): Promise<T> {
        let targetUrl = this.baseUrl + url;

        try {
            const {data, status} = await axios.post<T>(
                targetUrl,
                null,
                {
                    headers: {
                        isAnonymous: true,
                        'Content-Type': 'application/json'
                    }
                }
            )

            return data;

        } catch (error) {
            if (axios.isAxiosError(error)) {
                Logger.error("Axios error happened!", error.message);
                return null;
            } else {
                Logger.error("Unexpected error happened!", error);
                return null;
            }
        }
    }

    async login():Promise<LoginResponse> {
        let loginUrl = "/login?username=" + userInfo.username + "&password=" + userInfo.password
        let loginResponse:LoginResponse = await this._callApi<LoginResponse>(loginUrl)

        if(loginResponse.httpStatus && loginResponse.httpStatus == "OK"){
            userInfo.isLoggedIn = true
        }

        return loginResponse
    }

    async createAnonymousUser() {
        let createUserResponse: CreateUserResponse = await this._callApi<CreateUserResponse>("/users");

        window.localStorage.setItem("username", createUserResponse.username)
        window.localStorage.setItem("password", createUserResponse.password)

        userInfo.username = createUserResponse.username
        userInfo.password = createUserResponse.password
        userInfo.isLoggedIn = false
    }
}

let api = new RestApi()
export {api, LoginResponse}