import {HHForm} from "./HHForm"
import {createRoot, Root} from "react-dom/client"
import * as React from "react"

// Do I need to write a more robust/flexible UI framework?? Sounds like another big project!
class FormManager {
    currentForm: HHForm = null

    // Form class name=>Form object map
    formMap: Map<string, HHForm> = new Map()
    reactRootDiv: Root = null
    containerDiv: HTMLDivElement

    openReactForm(reactFormConstructor, props: any = null) {
        if (this.reactRootDiv == null) {
            this.containerDiv = document.createElement("div")
            this.containerDiv.style.position = "absolute"
            this.containerDiv.style.display = "block"
            this.containerDiv.style.top = "50%"
            this.containerDiv.style.left = "50%"
            this.containerDiv.style.transform = "translate(-50%, -50%)"

            document.body.appendChild(this.containerDiv)

            this.reactRootDiv = createRoot(this.containerDiv)
        }

        if (props == null) {
            props = {}
        }

        let _containerDiv = this.containerDiv
        props.closeForm = ()=>{
            _containerDiv.style.display = "none"
        }

        _containerDiv.style.display = "block"
        let formNode = React.createElement(reactFormConstructor, props)

        this.reactRootDiv.render(formNode)

        return formNode
    }

    openForm<FormType extends HTMLElement & HHForm>(formConstructor: new () => FormType): FormType {
        let form: FormType
        let formType = formConstructor.name

        if (typeof this.currentForm === formType) // The form has already been opened
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