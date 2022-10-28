import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";
import {SVGFiles} from "../Utilities/Svgs";

class ArrayPropertyDesc extends BasePropertyDesc{
    valueElementList: Array<HTMLElement> = new Array<HTMLElement>()
    constructor(property: Property) {
        super(property);

        let propertyListDiv = document.createElement("div")
        this.contentDiv.appendChild(propertyListDiv)

        let addButton = document.createElement("button")
        addButton.innerText = "+"
        propertyListDiv.appendChild(addButton)
    }
    onValueChanged(val) {
    }
}


class ArrayPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ArrayPropertyDesc(property)
    }
}

let arrayPropertyDivGenerator = new ArrayPropertyDivGenerator()

export {arrayPropertyDivGenerator}