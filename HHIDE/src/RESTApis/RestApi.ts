import {Logger} from "hhcommoncomponents"
import axios from "axios";
import {userInfo} from "../Identity/UserInfo";
import huahuoProperties from "/dist/hhide.properties";
import {HHToast} from "hhcommoncomponents";
import {LoginControllerApi, LoginStatus} from "../../dist/clientApi/index"

class CreateUserResponse {
    username: string
    password: string
}

class UserExistResponse {
    userName: string
    exist: boolean
}

enum HTTP_METHOD {
    POST,
    GET,
    PUT = 2,
    DELETE
}

// TODO: Use Swagger to generate the API class
class RestApi {
    baseUrl: string;

    getJwtToken(){
        let token = userInfo.jwtToken
        if (token == null) {
            Logger.error("Token is null, login again!")
            HHToast.error("Token is null, please re-login!")
        }
        return token
    }

    loginController
    constructor() {
        this.baseUrl = huahuoProperties["huahuo.backend.url"]
        this.loginController = new LoginControllerApi(undefined, this.baseUrl, axios)
    }

    getProjectPreviewImageUrl(projectId){
        let previewURLTemplate = `/projects/${projectId}/coverPage`
        return this.baseUrl + previewURLTemplate
    }

    async _callApi<T>(url: string, inHeaders: Object = null, requestBody: object = null, httpMethod: HTTP_METHOD = HTTP_METHOD.POST, isBlob: boolean = false): Promise<T> {
        let targetUrl = this.baseUrl + url;

        if (inHeaders == null) {
            inHeaders = {}
        }

        if (!inHeaders["Content-Type"]) {
            if(typeof requestBody === "string")
                inHeaders["Content-Type"] = "text/plain"
            else
                inHeaders["Content-Type"] = "application/json"
        }

        try {
            let config = {headers: {}}

            if(isBlob){
                config["responseType"] = 'blob'
            }

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
                case HTTP_METHOD.PUT:
                    returnObj = await axios.put<T>(
                        targetUrl,
                        requestBody,
                        config
                    )
                    break;
                case HTTP_METHOD.DELETE:
                    returnObj = await axios.delete<T>(
                        targetUrl,
                        config
                    )
                    break;
                }

            return returnObj["data"];

        } catch (error) {
            if (axios.isAxiosError(error)) {
                if (error.response.status == 401) {
                    Logger.error("Auth failed", error.message) // TODO: Relogin the user. TODO: Rememeber me.
                    HHToast.error("Auth failed! Invalid username/pwd!")
                    userInfo.logout()
                } else {
                    Logger.error("Axios error happened!", error.message);
                    HHToast.error("Axios error happened!" + error.message)
                }
                return null;
            } else {
                Logger.error("Unexpected error happened!", error);
                HHToast.error("Unexpected error happened during Api call!")
                return null;
            }
        }
    }

    async login(): Promise<LoginResponse> {
        let loginResponseRaw = await this.loginController.loginUsingPOST(userInfo.username, userInfo.password)
        let loginResponse:LoginResponse = loginResponseRaw.data

        if (loginResponse != null&& loginResponse.httpStatus && loginResponse.httpStatus == "OK") {
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

    async uploadFile(uploadPath:string, data: Blob, fileName: string){
        let headers = {
            "Authorization": "Bearer " + this.getJwtToken()
        };

        let formData = new FormData()
        data["lastModifiedDate"] = new Date();
        data["name"] = fileName;

        formData.append("file", data, fileName)
        headers["Content-Type"] = "multipart/form-data"

        return this._callApi(uploadPath, headers, formData)
    }

    async uploadProject(data: Blob, fileName: string, isElement: boolean = false) {
        let uploadPath = "/projects/projectData?isElement=" + isElement
        return this.uploadFile(uploadPath, data, fileName)
    }

    async uploadProjectCoverPage(projectId, data:Blob, fileName){
        let uploadPath = "/projects/" + projectId + "/coverPage"
        return this.uploadFile(uploadPath, data, fileName)
    }

    async downloadProject(projectId){
        let downloadPath = "/projects/" + projectId
        return this._callApi(downloadPath, null, null, HTTP_METHOD.GET, true)
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

    async listProjects(callBack: Function, pageNo: number = 0, pageSize: number = 10) {

        let headers = {
            "Authorization": "Bearer " + this.getJwtToken()
        };
        let listProjectApi = "/projects"
        let apiCallPromise = this._callApi(listProjectApi, headers, null, HTTP_METHOD.GET)
        apiCallPromise.then((projects)=>{
            callBack(projects)
        }).catch((ex)=>{
            HHToast.error("Exception happened when listing projects!" + ex)
        })
    }

    async listElements(callBack: Function, pageNo: number = 0, pageSize: number = 10) {

        let headers = {
            "Authorization": "Bearer " + this.getJwtToken()
        };
        let listElementApi = "/projects?isElement=true"
        let apiCallPromise = this._callApi(listElementApi, headers, null, HTTP_METHOD.GET)
        apiCallPromise.then((projects)=>{
            callBack(projects)
        }).catch((ex)=>{
            HHToast.error("Exception happened when listing projects!" + ex)
        })
    }

    async updateProjectDescription(projectId, description){
        let headers = {
            "Authorization": "Bearer " + this.getJwtToken()
        };
        let updateProjectDescriptionApi = "/projects/" + projectId + "/description"
        return this._callApi(updateProjectDescriptionApi, headers, description, HTTP_METHOD.PUT )
    }

    async checkProjectNameExistence(projectName) {
        let headers = {
            "Authorization": "Bearer " + this.getJwtToken()
        };
        let checkProjectNameExistenceURL = "/projects/exist?projectName=" + projectName
        return this._callApi(checkProjectNameExistenceURL, headers, null, HTTP_METHOD.GET )
    }

    async deleteProject(projectId){
        let headers = {
            "Authorization": "Bearer " + this.getJwtToken()
        };
        let deleteProjectURL = "/projects/"+projectId
        return this._callApi(deleteProjectURL, headers, null, HTTP_METHOD.DELETE )
    }
}

let api = new RestApi()
export {api, LoginResponse}