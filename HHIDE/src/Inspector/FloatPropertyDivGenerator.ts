import {BasePropertyDivGenerator, RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {PropertyType, Property, Vector2} from "hhcommoncomponents"

class FloatPropertyDiv{
    contentDiv: HTMLDivElement
    input : HTMLInputElement
    setter: Function

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
        property.registerValueChangeFunc(this.onValueChanged.bind(this))

        this.contentDiv = document.createElement("div")

        let currentValue = property.getter()
        this.setter = property.setter
        this.createInput(currentValue)

        this.contentDiv.appendChild(this.input)
    }

}

class FloatPropertyDivGenerator extends BasePropertyDivGenerator{

    generateDiv(property: Property): HTMLElement {
        let floatPropertyDiv = new FloatPropertyDiv(property)
        return floatPropertyDiv.contentDiv
    }
}

let floatPropertyDivGenerator = new FloatPropertyDivGenerator()
export {floatPropertyDivGenerator}