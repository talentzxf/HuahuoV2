import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "./CSSUtils";
import {HHForm} from "../Utilities/HHForm";
import {api} from "../RESTApis/RestApi"

@CustomElement({
    selector: "hh-project-list"
})
class ProjectListForm extends HTMLElement implements HHForm{
    projectListDiv:HTMLElement
    selector: string;
    closeBtn: HTMLElement
    projectListUlContainer: HTMLDivElement
    projectListUL:HTMLUListElement
    projectPageSpan: HTMLSpanElement

    connectedCallback(){
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

    closeForm(){
        this.style.display = "none"
    }

    updateProjectList(totalPage, curPageNo, projects){
        let ulInnerHTML = ""
        let projectDivPrefix = "projectDiv_"
        for(let project of projects){
            ulInnerHTML += "<li>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: row; flex-grow: 8' id='" + projectDivPrefix + project.id +"'>"
            ulInnerHTML += "    <div>"
            ulInnerHTML += "        <img style='border: 1px solid blue; width: 160px; height: 120px; object-fit: scale-down' src='" + api.getProjectPreviewImageUrl(project.id) + "'>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "    <div style='display: flex; flex-direction: column; width: 100%'>"
            ulInnerHTML += "        <span>" + project.name +"</span>"
            ulInnerHTML += "        <span style='font-size: x-small; text-align: right'>" + project.createTime.split("T")[0] +"</span>"
            ulInnerHTML += "        <span style='font-size: small'>" + project.description +"</span>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "    </div>"
            ulInnerHTML += "</li>"
        }

        this.projectListUL.innerHTML = ulInnerHTML

        for(let project of projects){
            let projectDiv = this.projectListUL.querySelector("#" + projectDivPrefix + project.id)
            projectDiv.addEventListener("mousedown", this.onClicked(project.id).bind(this))
        }
    }

    onClicked(projectId){
        return function onProjectClicked(evt){
            console.log("Clicked project:" + projectId + " evt:" + evt)
        }
    }
}

export {ProjectListForm}