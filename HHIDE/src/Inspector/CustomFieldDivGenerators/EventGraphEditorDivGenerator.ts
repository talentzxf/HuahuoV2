import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {EventGraphComponent} from "hhenginejs";
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
        button.value = i18n.t("edit")

        return button
    }
}

registerCustomFieldContentDivGeneratorConstructor(EventGraphComponent.name, "eventGraph", EventGraphEditorDivGenerator)
export {EventGraphEditorDivGenerator}