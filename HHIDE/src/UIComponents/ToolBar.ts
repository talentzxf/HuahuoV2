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

    createButton(svgFile, title, onClick){
        let btn = document.createElement("button")
        btn.className = "btn btn-outline-secondary"
        btn.style.width = "40px"
        btn.style.height = "40px"
        btn.innerHTML = svgFile
        btn.title = title
        btn.addEventListener("click", onClick)

        this.appendChild(btn)
    }

    constructor() {
        super();


        this.className = "btn-group btn-group-sm"

        i18n.ExecuteAfterInited(function () {
            this.saveButton = this.createButton(SVGFiles.saveBtn, i18n.t("hint.saveLocal"), this.save.bind(this))

            this.loadButton = this.createButton(SVGFiles.loadBtn, i18n.t("hint.loadLocal"), this.onFileSelected.bind(this))

            this.previewButton = this.createButton(SVGFiles.previewBtn, i18n.t("hint.preview"), this.uploadAndOpenPlayer.bind(this))

            this.elementListButton = this.createButton(SVGFiles.elementListButton, i18n.t("hint.library"), ()=>{
                this.listElements()
            })

            this.projectListButton = this.createButton(SVGFiles.projectListBtn, i18n.t("hint.listProject"), ()=>{
                this.listProjects()
            })

            this.uploadButton = this.createButton(SVGFiles.uploadBtn, i18n.t("hint.uploadProject"), ()=>{
                this.uploadProject()
            })
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
                        afterAction(response.data)
                })
            }
        } else {
            binaryFileUploader.upload().then((response) => {
                if (afterAction)
                    afterAction(response.data)
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
            form.setTitle(i18n.t("yourProjects"))
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

            form.setTitle(i18n.t("allElements"))
        })
    }
}

export {HHToolBar}