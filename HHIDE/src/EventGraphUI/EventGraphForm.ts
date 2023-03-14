import {HHForm} from "../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";

@CustomElement({
    selector: "hh-event-graph-form"
})
class EventGraphForm extends HTMLElement implements HHForm {
    selector: string;
    containerDiv: HTMLDivElement
    closeBtn: HTMLDivElement

    closeForm() {
        this.style.display = "none"
    }

    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"
        this.containerDiv = document.createElement("div")
        this.containerDiv.innerHTML = CSSUtils.formStyle

        this.containerDiv.innerHTML += "<form id='eventGraphContainer'> " +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='closeBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "</form>"

        let form = this.containerDiv.querySelector("form")
        this.closeBtn = this.containerDiv.querySelector("#closeBtn")
        this.closeBtn.onclick = this.closeForm.bind(this)

        this.appendChild(this.containerDiv)
    }

}

export {EventGraphForm}