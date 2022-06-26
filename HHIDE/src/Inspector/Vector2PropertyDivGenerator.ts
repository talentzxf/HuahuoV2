import {BasePropertyDesc, BasePropertyDivGenerator, RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {PropertyType, Property, Vector2} from "hhcommoncomponents"

class Vector2PropertyDesc extends BasePropertyDesc{
    inputX : HTMLInputElement
    inputY : HTMLInputElement

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

    constructor(property: Property) {
        super(property)

        let currentValue:Vector2 = property.getter()
        this.createVector2Divs(currentValue.x, currentValue.y)
    }

}

class Vector2PropertyDivGenerator extends BasePropertyDivGenerator{

    generatePropertyDesc(property): BasePropertyDesc {
        let vector2PropertyDiv = new Vector2PropertyDesc(property)
        return vector2PropertyDiv
    }
}

let vector2PropertyGenerator = new Vector2PropertyDivGenerator()
export {vector2PropertyGenerator}