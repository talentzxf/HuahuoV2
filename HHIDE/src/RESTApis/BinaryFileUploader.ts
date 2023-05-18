import {api} from "./RestApi";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {HHToast} from "hhcommoncomponents";
import {projectInfo} from "../SceneView/ProjectInfo";
import {projectManager} from "../HuaHuoEngine/ProjectManager";

class ProjectUploader {
    private get fileName(): string {
        return projectInfo.name
    }

    @NeedLogin()
    upload() {
        return this._uploadProject()
    }

    async _uploadProject() {
        if (!projectInfo.inited) {
            return
        }

        let data = projectManager.getProjectData()
        let uploadProjectPromise = api.uploadProject(data, this.fileName)

        return uploadProjectPromise.then((response) => {
            if (response != null && response["data"] && response["data"]["succeeded"]) {
                projectInfo.updateCoverPage()

                let promised = []
                // Two more things after project file has been uploaded.
                // 1. Update the project description.
                // 2. Update the project cover page.
                if (projectInfo.description && projectInfo.description.length > 0) {
                    let updateDescriptionPromise = api.updateProjectDescription(response["data"]["fileId"], projectInfo.description)
                    promised.push(updateDescriptionPromise)
                }

                let uploadCoverpagePromise = api.uploadProjectCoverPage(response["data"]["fileId"], projectInfo.coverPage, projectInfo.name + ".png")
                promised.push(uploadCoverpagePromise)

                Promise.all(promised).then(() => {
                    HHToast.info(i18n.t("toast.projectUploaded"))
                })
            } else {
                HHToast.error("Project upload failed")
            }
            return response
        })
    }
}

let binaryFileUploader = new ProjectUploader()
export {binaryFileUploader}