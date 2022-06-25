import {BasePropertyDivGenerator, RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {PropertyType, Property, Vector2} from "hhcommoncomponents"

class Vector2PropertyDivGenerator extends BasePropertyDivGenerator{
    contentDiv: HTMLDivElement

    getVector2HTMLText(x,y){
        let returnString = "<input style='width:50px' value='" + x + "'>"
        returnString += "<input style = 'width:50px' value='" + y + "'>"
        return returnString
    }

    onValueChanged(pos){
        this.contentDiv.innerHTML = this.getVector2HTMLText(pos.x, pos.y)
    }

    generateDiv(property: Property): HTMLElement {
        property.registerValueChangeFunc(this.onValueChanged.bind(this))

        let resultDiv = document.createElement("div")

        let currentValue:Vector2 = property.getter()

        resultDiv.innerHTML = this.getVector2HTMLText(currentValue.x, currentValue.y)
        this.contentDiv = resultDiv
        return resultDiv
    }
}

let vector2PropertyGenerator = new Vector2PropertyDivGenerator()
export {vector2PropertyGenerator}