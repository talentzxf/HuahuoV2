import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class ButtonPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        let button = document.createElement("button")
        button.onclick = property.config.action
        button.innerText = i18n.t(property.key)
        this.contentDiv.appendChild(button)
    }

    getTitleDiv(): HTMLElement {
        return null;
    }
}

class ButtonPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ButtonPropertyDesc(property);
    }
}

let buttonPropertyDivGenerator = new ButtonPropertyDivGenerator()
export {buttonPropertyDivGenerator}