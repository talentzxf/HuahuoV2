import {BasePropertyDivGenerator, RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {PropertyType, Property, Vector2} from "hhcommoncomponents"

class Vector2PropertyDivGenerator extends BasePropertyDivGenerator{
    contentDiv: HTMLDivElement
    inputX : HTMLInputElement
    inputY : HTMLInputElement
    setter: Function

    createInput(val):HTMLInputElement{
        let input = document.createElement("input")
        input.style.width = "50px"
        input.value = val
        input.type = "number"
        input.addEventListener("change", this.inputValueChanged.bind(this))
        return input
    }

    createVector2Divs(x,y){
        this.inputX = this.createInput(x)
        this.inputY = this.createInput(y)

        this.contentDiv.appendChild(this.inputX)
        this.contentDiv.appendChild(this.inputY)
    }

    inputValueChanged(){
        if(this.setter)
            this.setter(Number(this.inputX.value), Number(this.inputY.value))
    }

    onValueChanged(pos){
        this.inputX.value = pos.x
        this.inputY.value = pos.y
    }

    generateDiv(property: Property): HTMLElement {
        property.registerValueChangeFunc(this.onValueChanged.bind(this))

        this.contentDiv = document.createElement("div")

        let currentValue:Vector2 = property.getter()
        this.setter = property.setter

        this.createVector2Divs(currentValue.x, currentValue.y)
        return this.contentDiv
    }
}

let vector2PropertyGenerator = new Vector2PropertyDivGenerator()
export {vector2PropertyGenerator}