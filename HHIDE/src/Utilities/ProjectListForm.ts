import {CustomElement, Logger} from "hhcommoncomponents";
import {CSSUtils} from "./CSSUtils";
import {HHForm} from "../Utilities/HHForm";
import {api} from "../RESTApis/RestApi"
import {huahuoEngine} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {projectManager} from "../HuaHuoEngine/ProjectManager";
import {projectInfo} from "../SceneView/ProjectInfo";
import {formManager} from "./FormManager";

@CustomElement({
    selector: "hh-project-list"
})
class ProjectListForm extends HTMLElement implements HHForm {
    projectListDiv: HTMLElement
    selector: string;
    closeBtn: HTMLElement
    projectListUlContainer: HTMLDivElement
    projectListUL: HTMLUListElement

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

        this.projectListDiv = document.createElement("div")
        this.projectListDiv.innerHTML = CSSUtils.formStyle
        this.projectListDiv.innerHTML +=
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
        this.appendChild(this.projectListDiv)

        this.closeBtn = this.projectListDiv.querySelector("#projectListCloseBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.projectListUL = this.projectListDiv.querySelector("#projectListUl")

        this.projectListUlContainer = this.querySelector("#projectListUlContainer")
    }

    closeForm() {
        this.style.display = "none"
    }

    updateProjectList(totalPage, curPageNo, projects) {
        this.projectInfoMap.clear()

        this.projectListUL.innerHTML = i18n.t("project.nothing")

        let ulInnerHTML = ""
        let projectDivPrefix = "loadProject_"
        let deletProjectBtnPrefix = "deleteProject_"
        for (let project of projects) {
            ulInnerHTML += "<li>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: row; flex-grow: 8''>"
            ulInnerHTML += "    <div class='" + projectDivPrefix + project.id + "'>"
            ulInnerHTML += "        <img style='border: 1px solid blue; width: 160px; height: 120px; object-fit: scale-down' src='" + api.getProjectPreviewImageUrl(project.id) + "'>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: column; width: 100%'>"
            ulInnerHTML += "        <span >" + project.name + "</span>"
            ulInnerHTML += "        <span style='font-size: x-small; text-align: right' class='" + projectDivPrefix + project.id + "'>" + project.createTime.split("T")[0] + "</span>"
            ulInnerHTML += "        <span style='font-size: small'>" + project.description + "</span>"
            ulInnerHTML += "        <button id='" + deletProjectBtnPrefix + project.id + "'>" + i18n.t("project.delete") + "</button>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "</li>"

            this.projectInfoMap.set(project.id, project)
        }

        if(projects.length > 0)
            this.projectListUL.innerHTML = ulInnerHTML

        for (let project of projects) {
            let projectElements = this.projectListUL.querySelectorAll("." + projectDivPrefix + project.id)
            projectElements.forEach(ele => ele.addEventListener("mousedown", this.loadProject(project.id).bind(this)))

            let deleteProjectBtn = this.projectListUL.querySelector("#" + deletProjectBtnPrefix + project.id)
            deleteProjectBtn.addEventListener("click", this.deleteProject(project.id).bind(this))
        }
    }

    deleteProject(projectId) {
        let _this = this
        return function onProjectDelete(evt) {
            evt.preventDefault()
            evt.stopPropagation()

            let project: any = _this.projectInfoMap.get(projectId)
            let confirmResult = window.confirm("Are you sure to delete project:" + project.name + "?")
            if(confirmResult){
                Logger.info("Begin to delete project:" + projectId)
                api.deleteProject(projectId).then(()=>{
                    HHToast.info(i18n.t("project.deleted", {projectName: project.name}))
                    _this.refreshList()
                })
            }
        }
    }

    refreshList(pageNo = 0, pageSize = 10){
        let _this = this
        api.listProjects((listProjectResult)=>{
            let totalPage = listProjectResult.totalCount/pageSize
            _this.updateProjectList(totalPage, pageNo, listProjectResult.projectFiles)
        })
    }

    loadProject(projectId) {
        let _this = this
        return function onProjectClicked(evt) {
            if (!huahuoEngine.hasShape) {
                let project: any = _this.projectInfoMap.get(projectId)
                // Store is clean, directly load the project
                projectManager.loadFromServer(projectId).then(() => {
                    projectInfo.Setup(project.name, project.description, null)
                })
            } else { // Ask the user if he/she wants to clear the current store. TODO: Can we merge the two stores in the future??
                HHToast.warn("Can't merge project, not implemented!!!")
            }

            _this.closeForm()
        }
    }
}

export {ProjectListForm}