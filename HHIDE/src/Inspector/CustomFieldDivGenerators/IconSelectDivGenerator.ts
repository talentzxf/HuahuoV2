import {CustomFieldContentDivGenerator} from "hhcommoncomponents";
import {Particles} from "hhenginejs";
import {registerCustomFieldContentDivGeneratorConstructor} from "hhenginejs";

class IconSelectDivGenerator implements CustomFieldContentDivGenerator {
    particles: Particles

    constructor(particles) {
        this.particles = particles
    }

    generateDiv() {
        let div = document.createElement("div")
        div.innerHTML = "KKKKKKKKKKKKKKKKKK"

        return div
    }
}

// TODO: How to get the field name in a more elegant way ??
registerCustomFieldContentDivGeneratorConstructor(Particles.name, "particleShape")

export {IconSelectDivGenerator}