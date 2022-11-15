import {BasePropertyDivGenerator, BasePropertyDesc} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"
import {paper} from "hhenginejs"

import {HHColorInput} from "./InputComponents/HHColorInput";

class ColorPropertyDiv extends BasePropertyDesc{
    hhColorInput: HHColorInput

    onValueChanged(val:paper.Color){
        this.hhColorInput.value = val
    }

    constructor(property: Property) {
        super(property)

        this.hhColorInput = new HHColorInput(property.setter, property.getter, this.titleDiv)
        this.contentDiv.appendChild(this.hhColorInput)
    }
}

class ColorPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ColorPropertyDiv(property)
    }

    flexDirection(): string {
        return "column"
    }
}

let colorPropertyDivGenerator = new ColorPropertyDivGenerator()

export {colorPropertyDivGenerator}
