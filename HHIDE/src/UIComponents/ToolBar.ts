import {CustomElement} from "hhcommoncomponents";

import {SVGFiles} from "../Utilities/Svgs";
import {binaryFileUploader} from "../RESTApis/BinaryFileUploader";
import huahuoProperties from "/dist/hhide.properties";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {api} from "../RESTApis/RestApi"
import {ProjectListForm} from "./ProjectListForm";
import {formManager} from "../Utilities/FormManager";
import {ProjectInfoForm} from "./ProjectInfoForm";
import {projectInfo} from "../SceneView/ProjectInfo";
import {projectManager} from "../HuaHuoEngine/ProjectManager";

@CustomElement({
    selector: "hh-tool-bar"
})
class HHToolBar extends HTMLElement {
    saveButton: HTMLButtonElement
    loadButton: HTMLButtonElement
    previewButton: HTMLButtonElement
    projectListButton: HTMLButtonElement
    elementListButton: HTMLButtonElement
    uploadButton: HTMLButtonElement

    constructor() {
        super();

        i18n.ExecuteAfterInited(function () {
            this.saveButton = document.createElement("button")
            this.saveButton.style.width = "30px"
            this.saveButton.style.height = "30px"
            this.saveButton.innerHTML = SVGFiles.saveBtn
            this.saveButton.title = i18n.t("hint.saveLocal")
            this.saveButton.addEventListener("click", this.save.bind(this))
            this.appendChild(this.saveButton)

            this.loadButton = document.createElement("button")
            this.loadButton.style.width = "30px"
            this.loadButton.style.height = "30px"
            this.loadButton.title = i18n.t("hint.loadLocal")
            this.loadButton.innerHTML = SVGFiles.loadBtn

            this.loadButton.addEventListener("click", this.onFileSelected.bind(this))
            this.appendChild(this.loadButton)

            this.previewButton = document.createElement("button")
            this.previewButton.style.width = "30px"
            this.previewButton.style.height = "30px"
            this.previewButton.innerHTML = SVGFiles.previewBtn
            this.previewButton.title = i18n.t("hint.preview")
            this.previewButton.addEventListener("click", this.uploadAndOpenPlayer.bind(this))
            this.appendChild(this.previewButton)

            this.elementListButton = document.createElement("button")
            this.elementListButton.style.width = "30px"
            this.elementListButton.style.height = "30px"
            this.elementListButton.innerHTML = SVGFiles.elementListButton
            this.elementListButton.title = i18n.t("hint.library")
            let _this = this
            this.elementListButton.addEventListener("click", ()=>{
                _this.listElements()
            })
            this.appendChild(this.elementListButton)

            this.projectListButton = document.createElement("button")
            this.projectListButton.style.width = "30px"
            this.projectListButton.style.height = "30px"
            this.projectListButton.innerHTML = SVGFiles.projectListBtn
            this.projectListButton.title = i18n.t("hint.listProject")
            this.projectListButton.addEventListener("click", ()=>{
                _this.listProjects()
            })
            this.appendChild(this.projectListButton)

            this.uploadButton = document.createElement("button")
            this.uploadButton.style.width = "30px"
            this.uploadButton.style.height = "30px"
            this.uploadButton.innerHTML = SVGFiles.uploadBtn
            this.uploadButton.title = i18n.t("hint.uploadProject")

            this.uploadButton.onclick = function () {
                _this.uploadProject()
            }
            this.appendChild(this.uploadButton)

        }.bind(this))
    }

    @NeedLogin()
    uploadProject(afterAction: Function = null) {
        if (!projectInfo.inited) {
            // Prompt the Project description page.
            let storeInforForm = formManager.openForm(ProjectInfoForm)
            storeInforForm.onOKCallback = () => {
                binaryFileUploader.upload().then((response) => {
                    if (afterAction)
                        afterAction(response)
                })
            }
        } else {
            binaryFileUploader.upload().then((response) => {
                if (afterAction)
                    afterAction(response)
            })
        }
    }

    uploadAndOpenPlayer() {
        this.uploadProject((response) => {
            let fileId = response["fileId"]

            let playerUrl = huahuoProperties["huahuo.player.url"] + "?projectId=" + fileId

            window.open(playerUrl, '_blank')
        })
    }

    onFileSelected() {
        let hiddenFileButton = document.createElement("input")
        hiddenFileButton.type = "file"
        hiddenFileButton.click()

        hiddenFileButton.addEventListener("change", (evt) => {
            let fName = hiddenFileButton.value
            projectManager.load(fName, evt)
        })
    }

    save() {
        projectManager.save()
    }

    @NeedLogin()
    listProjects(pageNo: number = 0, pageSize: number = 10) {
        let _this = this
        api.listProjects((listProjectResult) => {
            let form = formManager.openForm(ProjectListForm)
            let totalPage = listProjectResult.totalCount / pageSize
            form.setUpdateListFunctor(_this.listProjects.bind(_this))
            form.updateList(totalPage, pageNo, pageSize, listProjectResult.binaryFiles)
        }, pageNo, pageSize)
    }

    deleteElement(elementId){

    }

    @NeedLogin()
    listElements(pageNo: number = 0, pageSize: number = 10) {
        let _this = this
        api.listElements(pageNo, pageSize).then((response) => {


            let listElementResult = response.data
            let form = formManager.openForm(ProjectListForm)
            let totalPage = listElementResult.totalCount / pageSize
            form.setUpdateListFunctor(_this.listElements.bind(_this))

            form.updateList(totalPage, pageNo, pageSize, listElementResult.binaryFiles, (elementId) => {
                projectManager.loadFromServer(elementId)
            }, true)
        })
    }
}

export {HHToolBar}