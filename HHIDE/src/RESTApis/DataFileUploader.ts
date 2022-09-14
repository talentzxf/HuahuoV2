import {huahuoEngine} from "hhenginejs";
import {SceneView} from "../SceneView/SceneView";
import {api} from "./RestApi";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {StoreInfoForm} from "../Utilities/StoreInfoForm";
import {formManager} from "../Utilities/FormManager";

declare var Module: any;

class DataFileUploader {

    private _fileName: string

    private get fileName():string{
        if(!this._fileName){
            // Generate a random file name for now.
            this._fileName = Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);
        }

        return this._fileName
    }

    @NeedLogin()
    upload() {
        return this._uploadProject()
    }

    getProjectData(): Blob {
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let oldStoreId = huahuoEngine.GetCurrentStoreId()

        try {
            console.log("Setting default store Id 3:" + mainSceneView.storeId)
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
            let Uint8Array = Module.writeObjectStoreInMemoryFile()
            let blob = new Blob([Uint8Array], {type: "application/octet-stream"})

            return blob
        } finally {
            console.log("Setting default store Id 4:" + oldStoreId)
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(oldStoreId)
        }
    }

    async _uploadProject() {
        let data = this.getProjectData()
        return api.uploadProject(data, this.fileName)
    }
}

let dataFileUploader = new DataFileUploader()
export {dataFileUploader}