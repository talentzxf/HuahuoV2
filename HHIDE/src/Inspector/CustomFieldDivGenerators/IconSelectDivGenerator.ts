import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {registerCustomFieldContentDivGeneratorConstructor} from "hhenginejs";
import {formManager} from "../../Utilities/FormManager";
import {SelectIconForm} from "./SelectIconForm";
import {Property} from "hhcommoncomponents";
import {AbstractComponent} from "hhenginejs";
import {Particles} from "hhenginejs";
import {HHRefreshableIconComponent} from "../InputComponents/HHRefreshableIconComponent";

class IconSelectDivGenerator implements CustomFieldContentDivGenerator{
    targetComponent: AbstractComponent

    handlerId: number = -1

    iconImage: HHRefreshableIconComponent

    constructor(targetComponent) {
        this.targetComponent = targetComponent
    }

    onSelectButtonClicked(){
        let selectIconForm = formManager.openForm(SelectIconForm)
        selectIconForm.onIconClicked = this.onIconClicked.bind(this)
    }

    onIconClicked(data){
        this.targetComponent.particleShape = data
    }

    generateDiv(property: Property) {
        if(this.handlerId > 0){
            property.unregisterValueChangeFunc(this.handlerId)
        }

        let fieldName = property.config.fieldName
        let div = document.createElement("div")

        let _this = this
        this.iconImage = new HHRefreshableIconComponent(()=>{
            return [_this.targetComponent, fieldName]
        })

        this.handlerId = property.registerValueChangeFunc(this.iconImage.refresh.bind(this.iconImage))

        this.iconImage.style.width = "30px"
        this.iconImage.style.height = "30px"
        div.appendChild(this.iconImage)

        let button = document.createElement("input")
        button.type = "button"
        button.value = i18n.t("inspector.select")
        button.onclick = this.onSelectButtonClicked.bind(this)

        div.appendChild(button)

        return div
    }
}

// TODO: How to get the field name in a more elegant way ??
registerCustomFieldContentDivGeneratorConstructor(Particles.name, "particleShape", IconSelectDivGenerator)

export {IconSelectDivGenerator}