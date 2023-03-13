import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {EventGraphComponent} from "hhenginejs";
import {IconSelectDivGenerator} from "./IconSelectDivGenerator";
import {registerCustomFieldContentDivGeneratorConstructor} from "hhenginejs";
import {Property} from "hhcommoncomponents";

class EventGraphEditorDivGenerator implements CustomFieldContentDivGenerator{
    targetComponent: EventGraphComponent

    constructor(targetComponent) {
        this.targetComponent = targetComponent
    }

    generateDiv(property: Property){
        let button = document.createElement("input")
        button.type = "button"
        button.innerText = i18n.t("editEventGraph")

        return button
    }
}

registerCustomFieldContentDivGeneratorConstructor(EventGraphComponent.name, "editEventGraph", IconSelectDivGenerator)
export {EventGraphEditorDivGenerator}