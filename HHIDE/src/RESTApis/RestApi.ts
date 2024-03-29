import {Logger} from "hhcommoncomponents"
import axios from "axios";
import {userInfo} from "../Identity/UserInfo";
import huahuoProperties from "/dist/hhide.properties";
import {HHToast} from "hhcommoncomponents";

import {
    LoginControllerApi, UserControllerApi, BinaryFileControllerApi,
    LoginStatus, UserDB, UserDBRoleEnum, UserDBStatusEnum,
    ElementControllerApi
} from "../../dist/clientApi/index"
import {huahuoEngine} from "hhenginejs";


// TODO: Use Swagger to generate the API class
class RestApi {
    baseUrl: string;

    getJwtToken() {
        let token = userInfo.jwtToken
        if (token == null) {
            Logger.error("Token is null, login again!")
            HHToast.error("Token is null, please re-login!")
        }
        return token
    }

    loginController
    userController
    fileController
    elementController

    constructor() {
        this.baseUrl = huahuoProperties["huahuo.backend.url"]
        this.loginController = new LoginControllerApi(undefined, this.baseUrl, axios)
        this.userController = new UserControllerApi(undefined, this.baseUrl, axios)
        this.fileController = new BinaryFileControllerApi(undefined, this.baseUrl, axios)
        this.userController = new UserControllerApi(undefined, this.baseUrl, axios)
        this.elementController = new ElementControllerApi(undefined, this.baseUrl, axios)
    }


    async login(): Promise<LoginStatus> {
        let loginResponseRaw = await this.loginController.login(userInfo.username, userInfo.password)
        let loginResponse: LoginStatus = loginResponseRaw.data

        if (loginResponse != null && loginResponseRaw.status == 200) {
            userInfo.jwtToken = loginResponse.jwtToken
            userInfo.isLoggedIn = true
        }

        return loginResponse
    }

    async createAnonymousUser() {

        let createUserResponseRaw = await this.userController.createUser(null, true)
        let createUserResponse: UserDB = createUserResponseRaw.data

        userInfo.username = createUserResponse.username
        userInfo.password = createUserResponse.password
        userInfo.isLoggedIn = false

        HHToast.info("Anonymous user:" + userInfo.username + " has been created")
    }

    getBinaryFileCoverPageUrl(fileId) {
        let previewURLTemplate = `/binaryfiles/${fileId}/coverPage`
        return this.baseUrl + previewURLTemplate
    }

    async createElement(fileName, isElement) {
        return this.elementController.createElement(fileName, isElement)
    }

    async uploadElement(data: Blob, fileName: string, storeId: string, isShareable = true, isEditable = true) {
        let engineVersion = huahuoEngine.getEngineVersion()
        return this.fileController.uploadFileForm(fileName, engineVersion, data, true, true, this.getAuthHeader()).then((response) => {
            if (response && response.data) {
                let fileId = response.data.fileId

                return this.elementController.createElement(storeId, fileId, isEditable, isShareable, this.getAuthHeader())
            }
        })
    }

    async uploadProject(data: Blob, fileName: string) {
        let engineVersion = huahuoEngine.getEngineVersion()
        return this.fileController.uploadFileForm(fileName, engineVersion, data, true, false, this.getAuthHeader())
    }

    async uploadProjectCoverPage(fileId, data: Blob, fileName, isElement: boolean = false) {
        return this.fileController.uploadCoverPageForm(fileName, fileId, data, isElement, this.getAuthHeader())
    }

    async downloadProject(fileId) {
        return this.fileController.downloadBinaryFile(fileId, {responseType: "arraybuffer"})
    }

    async isUserExist(username: string, existUserFunc: Function, userNotExistFunc: Function) {
        let userExistResponse = await this.userController.exist(username)
        let userExistResponseData = userExistResponse.data

        if (userExistResponseData.exist) {
            existUserFunc()
        } else {
            userNotExistFunc()
        }
    }

    async createUser(username: string, pwd: string, nickname: string, role = UserDBRoleEnum.CREATOR) {
        let userDB = {
            username: username,
            password: pwd,
            nickname: nickname,
            role: role,
            status: UserDBStatusEnum.ACTIVE
        }
        return this.userController.createUser(userDB, false)
    }

    async isTokenValid(username: string, jwtToken: string) {
        return this.loginController.isTokenValid(username, jwtToken)
    }

    getAuthHeader() {
        return {
            headers: {"Authorization": "Bearer " + userInfo.jwtToken}
        }
    }

    async listProjects(callBack: Function, pageNo: number = 0, pageSize: number = 10) {

        let apiCallPromise = this.fileController.listBinaryFiles(pageNo, pageSize, false, this.getAuthHeader())

        apiCallPromise.then((projects) => {
            callBack(projects.data)
        }).catch((ex) => {
            HHToast.error("Exception happened when listing projects!" + ex)
        })
    }

    async listElements(callBack: Function, pageNo: number = 0, pageSize: number = 10) {
        let apiCallPromise = this.fileController.listBinaryFiles(pageNo, pageSize, true, this.getAuthHeader())
        apiCallPromise.then((projects) => {
            callBack(projects.data)
        }).catch((ex) => {
            HHToast.error("Exception happened when listing elements!" + ex)
        })
    }

    async updateProjectDescription(fileId, description) {
        return this.fileController.updateBinaryFileDescription(fileId, description, this.getAuthHeader())
    }

    async checkFileNameExistence(fileName) {
        return this.fileController.existFile(fileName, this.getAuthHeader())
    }

    async deleteBinaryFile(fileId) {
        return this.fileController.deleteBinaryFile(fileId, this.getAuthHeader())
    }
}

let api = new RestApi()
export {api}