import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {Particles} from "hhenginejs";
import {registerCustomFieldContentDivGeneratorConstructor} from "hhenginejs";
import {formManager} from "../../Utilities/FormManager";
import {SelectIconForm} from "./SelectIconForm";

class IconSelectDivGenerator implements CustomFieldContentDivGenerator {
    particles: Particles

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

    generateDiv() {
        let div = document.createElement("div")
        let button = document.createElement("input")
        button.type = "button"
        button.value = i18n.t("inspector.select")
        button.onclick = this.onSelectButtonClicked

        div.appendChild(button)

        return div
    }
}

// TODO: How to get the field name in a more elegant way ??
registerCustomFieldContentDivGeneratorConstructor(Particles.name, "particleShape", IconSelectDivGenerator)

export {IconSelectDivGenerator}