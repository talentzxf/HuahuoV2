import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class ButtonPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        let button = document.createElement("button")
        button.onclick = property.action
        button.innerText = "Execute"
        this.contentDiv.appendChild(button)
    }

    onValueChanged(val) {
    }
}

class ButtonPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ButtonPropertyDesc(property);
    }
}

let buttonPropertyDivGenerator = new ButtonPropertyDivGenerator()
export {buttonPropertyDivGenerator}