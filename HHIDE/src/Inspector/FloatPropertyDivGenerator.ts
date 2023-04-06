import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"
import {HHFloatInput} from "./InputComponents/HHFloatInput";

class FloatPropertyDesc extends BasePropertyDesc{
    hhFloatInput: HHFloatInput

    constructor(property: Property) {
        super(property)

        let type = "number"
        if(property.config && property.config.elementType){
            type = property.config.elementType
        }

        this.hhFloatInput = new HHFloatInput(property.getter, property.setter, property.getKeyFrameCurve, type)

        if(property.config){
            if(property.config.min != null)
                this.hhFloatInput.min = property.config.min

            if(property.config.max != null)
                this.hhFloatInput.max = property.config.max

            if(property.config.step != null)
                this.hhFloatInput.step = property.config.step
        }

        this.contentDiv.appendChild(this.hhFloatInput)
    }

    onValueChanged(val) {
        this.hhFloatInput.value = val
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