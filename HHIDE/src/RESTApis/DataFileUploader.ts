import {userInfo} from "../Identity/UserInfo";
import {openLoginForm} from "../Identity/LoginForm";
import {huahuoEngine} from "hhenginejs";
import {SceneView} from "../SceneView/SceneView";
import {api} from "./RestApi";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
declare var Module:any;

class DataFileUploader{

    @NeedLogin()
    upload(){
        this._uploadProject()
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