import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class CustomFieldPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        if(property.config && property.config.contentGenerator)
            this.contentDiv.appendChild(property.config.contentGenerator.generateDiv(property))
    }

    onValueChanged(val) {
        console.log("value changed!")
    }

}

class CustomFieldDivGenerator extends BasePropertyDivGenerator{

    generatePropertyDesc(property): BasePropertyDesc {
        let customFieldPropertyDesc = new CustomFieldPropertyDesc(property)
        return customFieldPropertyDesc
    }
}

let customFieldDivGenerator = new CustomFieldDivGenerator()
export {customFieldDivGenerator}