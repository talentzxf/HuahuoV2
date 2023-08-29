import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {EventGraphComponent} from "hhenginejs";
import {Property} from "hhcommoncomponents";
import {formManager} from "../../Utilities/FormManager";
import {EventGraphForm} from "../../EventGraphUI/EventGraphForm";
import {registerCustomFieldContentDivGeneratorConstructor} from "../../ComponentProxy/ComponentProxy";

class EventGraphEditorDivGenerator implements CustomFieldContentDivGenerator{
    targetComponent: EventGraphComponent

    constructor(targetComponent) {
        this.targetComponent = targetComponent
    }

    openGraphEditor(){
        let eventGraphForm = formManager.openForm(EventGraphForm) as EventGraphForm
        eventGraphForm.setTargetComponent(this.targetComponent)
    }

    generateDiv(property: Property){
        let button = document.createElement("input")
        button.type = "button"
        button.value = i18n.t("edit")
        button.onclick = this.openGraphEditor.bind(this)

        return button
    }
}

registerCustomFieldContentDivGeneratorConstructor(EventGraphComponent.name, "eventGraph", EventGraphEditorDivGenerator)
export {EventGraphEditorDivGenerator}