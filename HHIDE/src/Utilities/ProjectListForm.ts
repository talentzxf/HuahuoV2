import {CustomElement} from "hhcommoncomponents";
import {CSSDefines} from "./CSSDefines";
import {HHForm} from "../Utilities/HHForm";
import {api} from "../RESTApis/RestApi"

@CustomElement({
    selector: "hh-project-list"
})
class ProjectListForm extends HTMLElement implements HHForm{
    projectListDiv:HTMLElement
    selector: string;
    closeBtn: HTMLElement
    projectListUL:HTMLUListElement

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
        this.projectListDiv.innerHTML = CSSDefines.formStyle
        this.projectListDiv.innerHTML +=
            "   <form id='projectListForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='projectListCloseBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "       <h3>Your Projects</h3>" +
            "       <ul id='projectListUl'></ul>"
            "   </form>"
        this.appendChild(this.projectListDiv)

        this.closeBtn = this.projectListDiv.querySelector("#projectListCloseBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.projectListUL = this.projectListDiv.querySelector("#projectListUl")


    }

    closeForm(){
        this.style.display = "none"
    }

    updateProjectList(projects){
        let ulInnerHTML = ""
        for(let project of projects){
            ulInnerHTML += "<li>"
            ulInnerHTML += "<span>" + project.name +"</span>"
            ulInnerHTML += "<img src='" + api.getProjectPreviewImageUrl(project.id) + "'>"
            ulInnerHTML += "</li>"
        }

        this.projectListUL.innerHTML = ulInnerHTML
    }
}

export {ProjectListForm}