import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"

class CustomFieldPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        if(property.config && property.config.contentDivGenerator)
            this.contentDiv.appendChild(property.config.contentDivGenerator.generateDiv())
    }

    onValueChanged(val) {
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