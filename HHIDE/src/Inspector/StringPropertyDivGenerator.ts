import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class StringPropertyDesc extends BasePropertyDesc{
    input: HTMLInputElement

    constructor(property: Property) {
        super(property);

        let currentValue:string = property.getter()

        if(!property.setter) // Create Read only properties
        {
            let div = document.createElement("span")
            div.innerText = i18n.t(currentValue)
            this.contentDiv.appendChild(div)
        }else{ // Create Input
            this.input = document.createElement("input")
            this.input.value = i18n.t(currentValue)
            this.input.addEventListener("keyup", this.inputValueChanged.bind(this))
            this.contentDiv.append(this.input)
        }
    }

    inputValueChanged(){
        let newValue = this.input.value
        this.property.setter(newValue)
    }
}

class StringPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        let stringPropertyDesc = new StringPropertyDesc(property)
        return stringPropertyDesc
    }
}

let stringPropertyDivGenerator = new StringPropertyDivGenerator()
export {stringPropertyDivGenerator}