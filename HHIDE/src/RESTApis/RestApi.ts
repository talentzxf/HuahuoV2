import {Logger} from "hhcommoncomponents"
import axios from "axios";
import {userInfo} from "../Identity/UserInfo";
import huahuoProperties from "../hhide.properties";
import {HHToast} from "hhcommoncomponents";

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

class UserExistResponse {
    userName: string
    exist: boolean
}

enum HTTP_METHOD {
    POST,
    GET
}

// TODO: Use Swagger to generate the API class
class RestApi {
    baseUrl: string;

    constructor() {
        this.baseUrl = huahuoProperties["huahuo.backend.url"]
    }

    async _callApi<T>(url: string, inHeaders: Object = null, requestBody: object = null, httpMethod: HTTP_METHOD = HTTP_METHOD.POST): Promise<T> {
        let targetUrl = this.baseUrl + url;

        if (inHeaders == null) {
            inHeaders = {}
        }

        if (!inHeaders["Content-Type"]) {
            inHeaders["Content-Type"] = "application/json"
        }

        try {

            let config = {headers: {}}

            if (inHeaders) {
                config.headers = inHeaders
            }

            let returnObj = {}
            switch (httpMethod) {

                case HTTP_METHOD.POST:
                    returnObj = await axios.post<T>(
                        targetUrl,
                        requestBody,
                        config
                    )
                    break;
                case HTTP_METHOD.GET:
                    returnObj = await axios.get<T>(
                        targetUrl,
                        config
                    )
                    break;
            }


            return returnObj["data"];

        } catch (error) {
            if (axios.isAxiosError(error)) {
                if(error.response.status == 401){
                    Logger.error("Auth failed", error.message) // TODO: Relogin the user. TODO: Rememeber me.
                }else{
                    Logger.error("Axios error happened!", error.message);
                }
                return null;
            } else {
                Logger.error("Unexpected error happened!", error);
                return null;
            }
        }
    }

    async login(): Promise<LoginResponse> {
        let loginUrl = "/login?userName=" + userInfo.username + "&password=" + userInfo.password
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

        userInfo.username = createUserResponse.username
        userInfo.password = createUserResponse.password
        userInfo.isLoggedIn = false

        HHToast.info("Anonymous user:" + userInfo.username + " has been created")
    }

    async uploadProject(data: Blob, fileName: string) {
        let token = userInfo.jwtToken
        if (token == null) {
            Logger.error("Token is null, login again!")
            return false
        }

        let headers = {
            "Authorization": "Bearer " + token
        };

        let uploadPath = "/projects/upload"
        let formData = new FormData()
        data["lastModifiedDate"] = new Date();
        data["name"] = fileName;

        formData.append("file", data, fileName)
        headers["Content-Type"] = "multipart/form-data"

        return this._callApi(uploadPath, headers, formData)
    }

    async isUserExist(username: string, existUserFunc: Function, userNotExistFunc: Function) {
        let userExistPath = "/users/exist?username=" + username
        let userExistResponseData: UserExistResponse = await this._callApi(userExistPath, null, null, HTTP_METHOD.GET)
        if (userExistResponseData.exist) {
            existUserFunc()
        } else {
            userNotExistFunc()
        }
    }

    async createUser(username: string, pwd: string, nickname: string): Promise<CreateUserResponse> {
        let createUserPath = "/users"
        let requestBody = {
            username: username,
            password: pwd,
            nickname: nickname,
            role: "CREATOR",
            status: "ACTIVE"
        }

        return this._callApi(createUserPath, null, requestBody)
    }

    async isTokenValid(username: string, jwtToken: string) {
        let isTokenValidUrl = "/tokenValid?userName=" + username + "&jwtToken=" + jwtToken
        return this._callApi(isTokenValidUrl, null, null, HTTP_METHOD.GET)
    }
}

let api = new RestApi()
export {api, LoginResponse}