import {BasePropertyDesc, BasePropertyDivGenerator, GetPropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";

class PanelPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        let panelPropertyDiv = document.createElement("div")

        this.contentDiv.appendChild(panelPropertyDiv)

        // TODO: Avoid duplication with Inspector
        for(let childProperty of property.children){
            let divGenerator = GetPropertyDivGenerator(property.type)
            let propertyDesc = divGenerator.generatePropertyDesc(property)

            let propertyDiv = document.createElement("div")
            propertyDiv.style.flexDirection = divGenerator.flexDirection()
            propertyDiv.style.display = "flex"
            propertyDiv.style.width = "100%"
            propertyDiv.style.gap = "10px"

            propertyDiv.appendChild(propertyDesc.getTitleDiv())
            propertyDiv.appendChild(propertyDesc.getContentDiv())
            this.contentDiv.appendChild(propertyDiv)
        }
    }
    onValueChanged(val) {
    }
}

class PanelPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new PanelPropertyDesc(property);
    }
}

let panelPropertyDivGenerator = new PanelPropertyDivGenerator()
export {panelPropertyDivGenerator}