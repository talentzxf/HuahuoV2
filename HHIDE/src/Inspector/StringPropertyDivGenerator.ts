import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class StringPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        let currentValue:string = property.getter()

        if(!property.setter) // Create Read only properties
        {
            let div = document.createElement("span")
            div.innerText = currentValue
            this.contentDiv.appendChild(div)
        }
    }

    onValueChanged(val) {
    }


}

class StringPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        let stringPropertyDesc = new StringPropertyDesc(property)
        return stringPropertyDesc
    }
}

let stringPropertyDivGenerator = new StringPropertyDivGenerator()
export {stringPropertyDivGenerator}