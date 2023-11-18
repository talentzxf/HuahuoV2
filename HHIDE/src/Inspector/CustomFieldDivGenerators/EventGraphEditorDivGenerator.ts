import {CustomFieldContentDivGenerator, Property} from "hhcommoncomponents";
import {EventGraphComponent} from "hhenginejs";
import {formManager} from "../../Utilities/FormManager";
import {EventGraphForm} from "../../EventGraphUI/EventGraphForm";

class EventGraphEditorDivGenerator implements CustomFieldContentDivGenerator {
    targetComponent: EventGraphComponent

    constructor(targetComponent) {
        this.targetComponent = targetComponent
    }

    openGraphEditor() {
        let eventGraphForm = formManager.openForm(EventGraphForm) as EventGraphForm
        eventGraphForm.setTargetComponent(this.targetComponent)
    }

    generateDiv(property: Property) {
        let editButton = document.createElement("input")
        editButton.type = "button"
        editButton.value = i18n.t("edit")
        editButton.onclick = this.openGraphEditor.bind(this)
        return editButton
    }
}

// registerCustomFieldContentDivGeneratorConstructor(EventGraphComponent.name, "eventGraph", EventGraphEditorDivGenerator)
export {EventGraphEditorDivGenerator}