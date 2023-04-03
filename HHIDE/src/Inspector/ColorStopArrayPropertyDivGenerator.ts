import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {HHColorStopArrayInput} from "./InputComponents/HHColorStopArray";

class ColorStopArrayPropertyDiv extends BasePropertyDesc{
    colorStopArrayDiv: HHColorStopArrayInput
    constructor(property) {
        super(property);

        this.colorStopArrayDiv = new HHColorStopArrayInput(property.getter, property.inserter, property.updater, property.deleter)
        this.contentDiv.appendChild(this.colorStopArrayDiv)
    }
}

class ColorStopArrayPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ColorStopArrayPropertyDiv(property)
    }

    flexDirection(): string {
        return "column"
    }
}

let colorStopArrayPropertyDivGenerator = new ColorStopArrayPropertyDivGenerator()

export {colorStopArrayPropertyDivGenerator}