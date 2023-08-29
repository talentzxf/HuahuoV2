import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class StringPropertyDesc extends BasePropertyDesc{
    input: HTMLSelectElement| HTMLInputElement

    constructor(property: Property) {
        super(property);

        let currentValue:string = property.getter()

        if(!property.setter) // Create Read only properties
        {
            this.contentDiv.innerText = i18n.t(currentValue)
            this.contentDiv.className = "input-group-text"
        }else{ // Create Input

            if(property.config && property.config.options && property.config.options.length > 0){
                this.input = document.createElement("select")
                this.input.className = "form-select"
                for(let option of property.config.options){
                    let optionEle = document.createElement("option")
                    optionEle.value = option
                    optionEle.innerText = option
                    this.input.appendChild(optionEle)
                }

                this.input.addEventListener("change", this.inputValueChanged.bind(this))

                this.contentDiv.append(this.input)
            }else{
                this.input = document.createElement("input") as HTMLInputElement
                (this.input as HTMLInputElement).value = i18n.t(currentValue)
                this.input.className = "form-control"
                this.input.addEventListener("keyup", this.inputValueChanged.bind(this))
                this.contentDiv.append(this.input)
            }
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