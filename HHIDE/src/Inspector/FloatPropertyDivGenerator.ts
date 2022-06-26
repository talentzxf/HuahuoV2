import {BasePropertyDesc, BasePropertyDivGenerator, RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {PropertyType, Property, Vector2} from "hhcommoncomponents"

class FloatPropertyDesc extends BasePropertyDesc{
    input : HTMLInputElement

    createInput(val):HTMLInputElement{
        this.input = document.createElement("input")
        this.input.style.width = "50px"
        this.input.value = val
        this.input.type = "number"
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
        this.createInput(currentValue)

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