import {HHForm} from "./HHForm"

// Do I need to write a more robust/flexible UI framework?? Sounds like another big project!
class FormManager {
    currentForm: HHForm = null

    // Form class name=>Form object map
    formMap: Map<string, HHForm> = new Map()

    openForm<FormType extends HTMLElement&HHForm>(formConstructor: new ()=>FormType):FormType {
        let form: FormType
        let formType = formConstructor.name

        if(typeof this.currentForm === formType) // The form has already been opened
            return this.currentForm as FormType

        if (this.currentForm) {
            this.currentForm.closeForm()
        }

        let needAppend = false

        form = this.formMap.get(formType) as FormType
        if (form == null) {
            // @ts-ignore
            let selector = formConstructor.selector
            form = document.createElement(selector) as FormType
            needAppend = true
            this.formMap.set(formType, form)
        }

        form.style.display = "block"
        this.currentForm = form

        if (needAppend) {
            document.body.appendChild(form)
        }
        return form
    }
}

let formManager = new FormManager()
export {formManager}