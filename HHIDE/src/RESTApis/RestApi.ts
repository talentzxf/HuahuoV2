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
    failReason: string
    jwtToken: string
    httpStatus: string
}

class RestApi {
    baseUrl: string;

    constructor() {
        this.baseUrl = huahuoProperties["huahuo.backend.url"]
    }

    async _callApi<T>(url: string, inHeaders: Object = null, requestData: Blob = null): Promise<T> {
        let targetUrl = this.baseUrl + url;

        let formData = null
        if(inHeaders == null){
            inHeaders = {}
        }

        if(requestData){
            // Generate a random file name for now.
            let fileName = Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);

            formData = new FormData()
            requestData["lastModifiedDate"] = new Date();
            requestData["name"] = fileName;

            formData.append("file",  requestData)
            inHeaders["Content-Type"] = "multipart/form-data"
        }else{
            inHeaders["Content-Type"] = "application/json"
        }

        try {

            let config = {headers:{}}

            if(inHeaders) {
                config.headers = inHeaders
            }

            const {data, status} = await axios.post<T>(
                targetUrl,
                formData,
                config
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

    async login(): Promise<LoginResponse> {
        let loginUrl = "/login?username=" + userInfo.username + "&password=" + userInfo.password
        let loginResponse: LoginResponse = await this._callApi<LoginResponse>(loginUrl)

        if (loginResponse.httpStatus && loginResponse.httpStatus == "OK") {
            userInfo.jwtToken = loginResponse.jwtToken
            userInfo.isLoggedIn = true
        }

        return loginResponse
    }

    async createAnonymousUser() {

        let headers = {
            isAnonymous: true
        };

        let createUserResponse: CreateUserResponse = await this._callApi<CreateUserResponse>("/users", headers);

        window.localStorage.setItem("username", createUserResponse.username)
        window.localStorage.setItem("password", createUserResponse.password)

        userInfo.username = createUserResponse.username
        userInfo.password = createUserResponse.password
        userInfo.isLoggedIn = false
    }

    async uploadProject(data: Blob): Promise<boolean> {
        let token = userInfo.jwtToken
        if (token == null) {
            Logger.error("Token is null, login again!")
            return false
        }

        let headers = {
            "Authorization": "Bearer " + token
        };

        let uploadPath = "/projects/upload"
        let responseData = this._callApi(uploadPath, headers, data)

        console.log(responseData)

        return true
    }
}

let api = new RestApi()
export {api, LoginResponse}