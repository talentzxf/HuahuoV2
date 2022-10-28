import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class FloatPropertyDesc extends BasePropertyDesc{
    input : HTMLInputElement

    createInput(val, type):HTMLInputElement{
        this.input = document.createElement("input")
        this.input.style.width = "50px"
        this.input.value = val
        this.input.type = type
        this.input.addEventListener("change", this.inputValueChanged.bind(this))
        return this.input
    }

    inputValueChanged(){
        if(this.setter)
            this.setter(Number(this.input.value))
    }

    onValueChanged(pos){
        this.input.value = pos
    }

    constructor(property: Property) {
        super(property)
        let currentValue = property.getter()
        let type = "number"
        if(property.config && property.config.elementType){
            type = property.config.elementType
        }

        this.createInput(currentValue, type)

        if(property.config){
            this.input.min = property.config.min
            this.input.max = property.config.max
            this.input.step = property.config.step
        }

        // update the current value again.
        this.input.value = currentValue

        this.contentDiv.appendChild(this.input)
    }

}

class FloatPropertyDivGenerator extends BasePropertyDivGenerator{

    generatePropertyDesc(property: Property): BasePropertyDesc {
        let floatPropertyDesc = new FloatPropertyDesc(property)
        return floatPropertyDesc
    }
}

let floatPropertyDivGenerator = new FloatPropertyDivGenerator()
export {floatPropertyDivGenerator}