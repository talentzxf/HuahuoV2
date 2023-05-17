import {CustomElement, Logger} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {HHForm} from "../Utilities/HHForm";
import {api} from "../RESTApis/RestApi"
import {huahuoEngine} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {projectManager} from "../HuaHuoEngine/ProjectManager";
import {projectInfo} from "../SceneView/ProjectInfo";

@CustomElement({
    selector: "hh-project-list"
})
class ProjectListForm extends HTMLElement implements HHForm {
    listDiv: HTMLElement
    selector: string;
    closeBtn: HTMLElement
    listUlContainer: HTMLDivElement
    listUL: HTMLUListElement

    projectInfoMap: Map<number, Object> = new Map

    connectedCallback() {
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"

        this.innerHTML += "<style>" +
            "ul{" +
            "list-style-type: none;" +
            "width: 500px" +
            "}" +
            "li img {" +
            "  float: left;" +
            "  margin: 0 15px 0 0;" +
            "}" +
            "</style>"

        this.listDiv = document.createElement("div")
        this.listDiv.innerHTML = CSSUtils.formStyle
        this.listDiv.innerHTML +=
            "   <form id='projectListForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='projectListCloseBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "       <h3>Your Projects</h3>" +
            "       <div id='projectListUlContainer' style='height: 500px; overflow-x: hidden; overflow-y: auto; width: 100%'>" +
            "           <ul id='projectListUl' style='width: 100%'></ul>" +
            "       </div>" +
            "   </form>"
        this.appendChild(this.listDiv)

        this.closeBtn = this.listDiv.querySelector("#projectListCloseBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.listUL = this.listDiv.querySelector("#projectListUl")

        this.listUlContainer = this.querySelector("#projectListUlContainer")
    }

    closeForm() {
        this.style.display = "none"
    }

    updateList(totalPage, curPageNo, projects, onItemClicked: Function = null, enableDeletion = true, writeAuthInfo = false) {
        this.projectInfoMap.clear()

        this.listUL.innerHTML = i18n.t("project.nothing")

        let ulInnerHTML = ""
        let projectDivPrefix = "loadProject_"
        let deletProjectBtnPrefix = "deleteProject_"
        for (let project of projects) {
            ulInnerHTML += "<li>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: row; flex-grow: 8''>"
            ulInnerHTML += "    <div class='" + projectDivPrefix + project.id + "'>"
            ulInnerHTML += "        <img style='border: 1px solid blue; width: 160px; height: 120px; object-fit: scale-down' src='" + api.getBinaryFileCoverPageUrl(project.id) + "'>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: column; width: 100%'>"
            ulInnerHTML += "        <span >" + project.name + "</span>"
            if(writeAuthInfo)
                ulInnerHTML += "    <span style='font-size: small; text-align: right'>" + project.createdBy + "</span>"
            ulInnerHTML += "        <span style='font-size: x-small; text-align: right' class='" + projectDivPrefix + project.id + "'>" + project.createTime.split("T")[0] + "</span>"
            ulInnerHTML += "        <span style='font-size: small'>" + project.description + "</span>"

            if (enableDeletion)
                ulInnerHTML += "        <button id='" + deletProjectBtnPrefix + project.id + "'>" + i18n.t("project.delete") + "</button>"

            ulInnerHTML += "    </div>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "</li>"

            this.projectInfoMap.set(project.id, project)
        }

        if (projects.length > 0)
            this.listUL.innerHTML = ulInnerHTML

        let _this = this
        let callBack = onItemClicked == null ? this.loadProject.bind(this) : onItemClicked

        for (let project of projects) {
            let projectElements = this.listUL.querySelectorAll("." + projectDivPrefix + project.id)
            projectElements.forEach(ele => ele.addEventListener("mousedown", (evt) => {
                callBack(project.id)
                _this.closeForm()
            }))

            if (enableDeletion) {
                let deleteProjectBtn = this.listUL.querySelector("#" + deletProjectBtnPrefix + project.id)
                deleteProjectBtn.addEventListener("click", this.deleteProject(project.id).bind(this))
            }
        }
    }

    deleteProject(projectId) {
        let _this = this
        return function onProjectDelete(evt) {
            evt.preventDefault()
            evt.stopPropagation()

            let project: any = _this.projectInfoMap.get(projectId)
            let confirmResult = window.confirm("Are you sure to delete project:" + project.name + "?")
            if (confirmResult) {
                Logger.info("Begin to delete project:" + projectId)
                api.deleteBinaryFile(projectId).then(() => {
                    HHToast.info(i18n.t("project.deleted", {projectName: project.name}))
                    _this.refreshList()
                })
            }
        }
    }

    refreshList(pageNo = 0, pageSize = 20) {
        let _this = this
        api.listProjects((listProjectResult) => {
            let totalPage = listProjectResult.totalCount / pageSize
            _this.updateList(totalPage, pageNo, listProjectResult.binaryFiles)
        })
    }

    loadProject(projectId) {
        if (!huahuoEngine.hasShape) {
            let project: any = this.projectInfoMap.get(projectId)
            // Store is clean, directly load the project
            projectManager.loadFromServer(projectId).then(() => {
                projectInfo.Setup(project.name, project.description, null)
            })
        } else { // Ask the user if he/she wants to clear the current store. TODO: Can we merge the two stores in the future??
            HHToast.warn("Can't merge project, not implemented!!!")
        }
    }
}

export {ProjectListForm}