import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {HHColorStopArrayInput} from "./InputComponents/HHColorStopArray";

class ColorStopArrayPropertyDiv extends BasePropertyDesc{
    colorStopArrayDiv: HHColorStopArrayInput

    onValueChanged(val) {
    }

    constructor(property) {
        super(property);

        this.colorStopArrayDiv = new HHColorStopArrayInput(property.getter, property.setter)
        this.contentDiv.appendChild(this.colorStopArrayDiv)
    }
}

class ColorStopArrayPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ColorStopArrayPropertyDiv(property)
    }
}

let colorStopArrayPropertyDivGenerator = new ColorStopArrayPropertyDivGenerator()

export {colorStopArrayPropertyDivGenerator}