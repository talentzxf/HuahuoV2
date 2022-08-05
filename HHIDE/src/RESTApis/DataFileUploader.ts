import {userInfo} from "../Login/UserInfo";
import {openLoginForm} from "../Login/LoginForm";
import {huahuoEngine} from "hhenginejs";
import {SceneView} from "../SceneView/SceneView";
import {api} from "./RestApi";
declare var Module:any;

class DataFileUploader{
    upload(){
        if(!userInfo.isLoggedIn){
            // Prompt login window
            let loginForm = openLoginForm(this._uploadProject.bind(this))
        }else{
            this._uploadProject()
        }
    }

    getProjectData():Blob{
        let mainSceneView:SceneView = document.querySelector("#mainScene")
        let oldStoreId = huahuoEngine.GetCurrentStoreId()

        try{
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
            let Uint8Array = Module.writeObjectStoreInMemoryFile()
            let blob = new Blob([Uint8Array], {type: "application/octet-stream"})

            return blob
        }finally {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(oldStoreId)
        }
    }

    _uploadProject(){
        let data = this.getProjectData()
        api.uploadProject(data)
    }
}

let dataFileUploader = new DataFileUploader()
export {dataFileUploader}