import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property, Vector3} from "hhcommoncomponents"
import {HHVector3Input} from "./InputComponents/HHVector3Input";

class Vector3PropertyDesc extends BasePropertyDesc{
    vector3Input: HHVector3Input

    onValueChanged(val){
        this.vector3Input.value = val
    }

    constructor(property: Property) {
        super(property)
        this.vector3Input = new HHVector3Input(property.getter, property.setter)
        this.contentDiv.appendChild(this.vector3Input)
    }
}

class Vector3PropertyDivGenerator extends BasePropertyDivGenerator{

    generatePropertyDesc(property): BasePropertyDesc {
        let vector2PropertyDiv = new Vector3PropertyDesc(property)
        return vector2PropertyDiv
    }
}

let vector3PropertyGenerator = new Vector3PropertyDivGenerator()
export {vector3PropertyGenerator}