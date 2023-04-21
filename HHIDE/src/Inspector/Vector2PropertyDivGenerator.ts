import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"
import {HHVector2Input} from "./InputComponents/HHVector2Input";

class Vector2PropertyDesc extends BasePropertyDesc{
    vector2Input: HHVector2Input

    onValueChanged(val){
        this.vector2Input.value = val
    }

    constructor(property: Property) {
        super(property)
        this.vector2Input = new HHVector2Input(property.getter, property.setter, property.config.getKeyFrameCurves)
        this.contentDiv.appendChild(this.vector2Input)
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