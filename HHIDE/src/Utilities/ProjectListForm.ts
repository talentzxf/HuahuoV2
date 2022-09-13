import {CustomElement} from "hhcommoncomponents";
import {CSSDefines} from "./CSSDefines";
import {HHForm} from "../Utilities/HHForm";

@CustomElement({
    selector: "hh-project-list"
})
class ProjectListForm extends HTMLElement implements HHForm{
    projectListDiv:HTMLElement
    selector: string;
    closeBtn: HTMLElement

    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"

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
            "   </form>"
        this.appendChild(this.projectListDiv)

        this.closeBtn = this.projectListDiv.querySelector("#projectListCloseBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))
    }

    closeForm(){
        this.style.display = "none"
    }
}

export {ProjectListForm}