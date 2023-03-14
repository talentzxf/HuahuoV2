import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {EventGraphComponent} from "hhenginejs";
import {registerCustomFieldContentDivGeneratorConstructor} from "hhenginejs";
import {Property} from "hhcommoncomponents";
import {formManager} from "../../Utilities/FormManager";
import {EventGraphForm} from "../../EventGraphUI/EventGraphForm";

class EventGraphEditorDivGenerator implements CustomFieldContentDivGenerator{
    targetComponent: EventGraphComponent

    constructor(targetComponent) {
        this.targetComponent = targetComponent
    }

    openGraphEditor(){
        formManager.openForm(EventGraphForm)
    }

    generateDiv(property: Property){
        let button = document.createElement("input")
        button.type = "button"
        button.value = i18n.t("edit")
        button.onclick = this.openGraphEditor

        return button
    }
}

registerCustomFieldContentDivGeneratorConstructor(EventGraphComponent.name, "eventGraph", EventGraphEditorDivGenerator)
export {EventGraphEditorDivGenerator}