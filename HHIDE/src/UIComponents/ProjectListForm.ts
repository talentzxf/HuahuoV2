import {CustomElement, Logger} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {HHForm} from "../Utilities/HHForm";
import {api} from "../RESTApis/RestApi"
import {huahuoEngine} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {projectManager} from "../HuaHuoEngine/ProjectManager";
import {projectInfo} from "../SceneView/ProjectInfo";
import {userInfo} from "../Identity/UserInfo";
import {BaseForm} from "./BaseForm";

@CustomElement({
    selector: "hh-project-list"
})
class ProjectListForm extends BaseForm {
    listDiv: HTMLElement
    listUlContainer: HTMLDivElement
    listUL: HTMLUListElement
    paginationDiv: HTMLDivElement

    projectInfoMap: Map<number, Object> = new Map

    titleText: string = "Projects"

    connectedCallback() {
        super.connectedCallback()

        this.modalTitle.innerText = this.titleText
        this.listDiv = document.createElement("div")
        this.listDiv.innerHTML +=
            "       <div id='projectListUlContainer' style='height: 500px; overflow-x: hidden; overflow-y: auto; width: 100%'>" +
            "           <ul id='projectListUl' style='width: 100%; float: left'></ul>" +
            "       </div>" +
            "       <div id='pagination' style='display: none'></div>"

        this.form.appendChild(this.listDiv)

        this.listUL = this.listDiv.querySelector("#projectListUl")
        this.paginationDiv = this.listDiv.querySelector("#pagination")

        this.listUlContainer = this.querySelector("#projectListUlContainer")
    }

    updateListFunctor:(totalPage, curPageNo)=>void
    setUpdateListFunctor(updateListFunctor:(totalPage, curPageNo)=>void){
        this.updateListFunctor = updateListFunctor
    }

    setTitle(title: string){
        this.titleText = title;
        this.modalTitle.innerText = this.titleText
    }

    updateList(totalPage, curPageNo, pageSize, projects, onItemClicked: Function = null, writeAuthInfo = false) {
        this.projectInfoMap.clear()

        this.listUL.innerHTML = i18n.t("project.nothing")

        let ulInnerHTML = ""
        let projectDivPrefix = "loadProject_"
        let deletProjectBtnPrefix = "deleteProject_"
        for (let project of projects) {
            ulInnerHTML += "<li style='float: left; padding: 10px; height: 245px'>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: column; flex-grow: 8''>"
            ulInnerHTML += "    <div class='" + projectDivPrefix + project.id + "'>"
            ulInnerHTML += "        <img style='border: 1px solid blue; width: 160px; height: 120px; object-fit: scale-down; margin: 0px' src='" + api.getBinaryFileCoverPageUrl(project.id) + "'>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: column; width: 100%'>"
            ulInnerHTML += "        <span >" + project.name + "</span>"
            if(writeAuthInfo)
                ulInnerHTML += "    <span style='font-size: small; text-align: right'>" + project.createdBy + "</span>"
            ulInnerHTML += "        <span style='font-size: x-small; text-align: right' class='" + projectDivPrefix + project.id + "'>" + project.createTime.split("T")[0] + "</span>"

            let description = project.description.length > 0? project.description:"No Description"
            ulInnerHTML += "        <span style='font-size: small'>" + description + "</span>"

            if (project.createdBy === userInfo.username)
                ulInnerHTML += "        <button id='" + deletProjectBtnPrefix + project.id + "'>" + i18n.t("project.delete") + "</button>"

            ulInnerHTML += "    </div>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "</li>"
            this.projectInfoMap.set(project.id, project)
        }

        if(totalPage > 1){
            let liPrefix = "listPage_"
            let innerHTML = "<ul class='pagination'>"
                for(let pageId = 0; pageId < totalPage; pageId++){
                    innerHTML += "<li class='page-item' id='" + liPrefix + pageId +"' style='padding: 10px; list-style-type: none; float: left'>"
                    if(pageId != curPageNo) // When displayed, add 1.
                        innerHTML += "<a class='page-link' href='#'>" + (pageId + 1) + "</a>"
                    else
                        innerHTML += pageId + 1

                    innerHTML +="</li>"
                }
            innerHTML += "</ul>"
            this.paginationDiv.innerHTML = innerHTML

            let _this = this
            for(let pageId = 0; pageId < totalPage; pageId++){
                let pageLiId = "#" + liPrefix + pageId
                let pageLi = this.paginationDiv.querySelector(pageLiId)
                pageLi.addEventListener("click", ()=>{
                    if(_this.updateListFunctor)
                        _this.updateListFunctor(pageId, pageSize)
                })
            }

            this.paginationDiv.style.display = "block"
        }else{
            this.paginationDiv.style.display = "none"
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

            let deleteProjectBtn = this.listUL.querySelector("#" + deletProjectBtnPrefix + project.id)
            if(deleteProjectBtn)
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
            if (confirmResult) {
                Logger.info("Begin to delete project:" + projectId)
                api.deleteBinaryFile(projectId).then(() => {
                    HHToast.info(i18n.t("project.deleted", {projectName: project.name}))

                    if(project.fileType == "ELEMENT"){
                        _this.refreshElementList()
                    } else if(project.fileType == "PROJECT"){
                        _this.refreshProjectList()
                    }else{
                        console.error("Unknown file type")
                    }
                })
            }
        }
    }

    refreshElementList(pageNo = 0, pageSize = 10){
        let _this = this
        api.listElements(0, 10).then((listElementResult) => {
            let listResultData = listElementResult.data
            let totalPage = listResultData.totalCount / pageSize
            _this.updateList(totalPage, pageNo, pageSize, listResultData.binaryFiles)
        })
    }

    refreshProjectList(pageNo = 0, pageSize = 10) {
        let _this = this
        api.listProjects((listProjectResult) => {
            let totalPage = listProjectResult.totalCount / pageSize
            _this.updateList(totalPage, pageNo, pageSize, listProjectResult.binaryFiles)
        }, pageNo, pageSize)
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