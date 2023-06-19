import {HHForm} from "../Utilities/HHForm";

abstract class BaseForm extends HTMLElement implements HHForm {
    selector: string;

    modalTitle: HTMLElement
    form: HTMLFormElement
    footer: HTMLElement

    closeForm() {
        this.style.display = "none";
    }

    connectedCallback() {
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"

        let formContainer = document.createElement("div")

        formContainer.innerHTML +=
            "<div class='modal' tabindex='-1' style='display:block; position: static'>" +
            "  <div class='modal-dialog'>" +
            "    <div class='modal-content'>" +
            "      <div class='modal-header'>" +
            "        <h5 class='modal-title'>Modal title</h5>" +
            "        <button type='button' class='btn-close' data-bs-dismiss='modal' aria-label='Close'></button>" +
            "      </div>" +
            "      <div class='modal-body'>" +
            "          <form></form>"+
            "      </div>" +
            "      <div class='modal-footer'>" +
            "      </div>" +
            "    </div>" +
            "  </div>" +
            "</div>"

        this.modalTitle = formContainer.querySelector(".modal-title")
        this.footer = formContainer.querySelector(".modal-footer")


        this.appendChild(formContainer)

        this.form = formContainer.querySelector("form")
        this.form.addEventListener("submit", function (e) {
            e.stopPropagation()
            e.preventDefault()
        })

       let closeBtns = formContainer.querySelectorAll("button[data-bs-dismiss='modal']")
        closeBtns.forEach((closeBtn)=>{
            closeBtn.addEventListener("mousedown", this.closeForm.bind(this))
        })
    }

}

export {BaseForm}