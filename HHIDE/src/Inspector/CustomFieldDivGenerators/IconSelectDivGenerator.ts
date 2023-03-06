import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {Particles} from "hhenginejs";
import {registerCustomFieldContentDivGeneratorConstructor} from "hhenginejs";
import {formManager} from "../../Utilities/FormManager";
import {SelectIconForm} from "./SelectIconForm";
import {Property} from "hhcommoncomponents";

class IconSelectDivGenerator implements CustomFieldContentDivGenerator {
    particles: Particles

    handlerId: number = -1

    iconImage: HTMLImageElement

    constructor(particles) {
        this.particles = particles
    }

    onSelectButtonClicked(){
        let selectIconForm = formManager.openForm(SelectIconForm)
        selectIconForm.onIconClicked = this.onIconClicked.bind(this)
    }

    onIconClicked(data){
        this.particles.particleShape = data
    }

    onValueChanged(data){
        if(this.iconImage)
            this.iconImage.src = data
        else
            console.warn("Icon image is null??")
    }

    generateDiv(property: Property) {
        if(this.handlerId > 0){
            property.unregisterValueChangeFunc(this.handlerId)
        }

        this.handlerId = property.registerValueChangeFunc(this.onValueChanged.bind(this))

        let div = document.createElement("div")
        this.iconImage = document.createElement("img")
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