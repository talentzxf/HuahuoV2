import {huahuoEngine} from "hhenginejs";
import {SceneView} from "../SceneView/SceneView";
import {api} from "./RestApi";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {HHToast} from "hhcommoncomponents";
import {projectInfo} from "../SceneView/ProjectInfo";

declare var Module: any;

class ProjectUploader {
    private get fileName():string{
        return projectInfo.name
    }

    @NeedLogin()
    upload() {
        return this._uploadProject()
    }

    getProjectData(): Blob {
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let oldStoreId = huahuoEngine.GetCurrentStoreId()

        try {
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
        if(!projectInfo.inited){
            return
        }

        let data = this.getProjectData()
        let uploadProjectPromise = api.uploadProject(data, this.fileName)

        return uploadProjectPromise.then((response)=>{
            if(response["succeeded"]){
                // Two more things after project file has been uploaded.
                // 1. Update the project description.
                // 2. Update the project cover page.
                let updateDescriptionPromise = api.updateProjectDescription(response["fileId"], projectInfo.description)
                let uploadCoverpagePromise = api.uploadProjectCoverPage(response["fileId"], projectInfo.coverPage, projectInfo.name + ".png")

                HHToast.info(i18n.t("toast.projectUploaded"))
            }
            return response
        })
    }
}

let projectUploader = new ProjectUploader()
export {projectUploader}