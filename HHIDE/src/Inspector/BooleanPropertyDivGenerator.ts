import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class BooleanPropertyDesc extends BasePropertyDesc{
    getter
    setter

    checkBox: HTMLInputElement
    constructor(property: Property) {
        super(property);
        this.checkBox = document.createElement("input")
        this.checkBox.type = "checkbox"

        this.setter = property.setter
        this.getter = property.getter

        this.checkBox.checked = this.getter()

        this.checkBox.addEventListener("change", this.inputValueChanged.bind(this))

        this.contentDiv.appendChild(this.checkBox)
    }

    inputValueChanged(){
        this.setter(this.checkBox.checked)
    }
}

class BooleanPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        let booleanPropertyDesc = new BooleanPropertyDesc(property)
        return booleanPropertyDesc;
    }
}

let booleanPropertyDivGenerator = new BooleanPropertyDivGenerator()
export {booleanPropertyDivGenerator}