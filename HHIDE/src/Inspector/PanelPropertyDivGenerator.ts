import {
    BasePropertyDesc,
    BasePropertyDivGenerator,
    GenerateDiv,
    GetPropertyDivGenerator
} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";

class PanelPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        let panelPropertyDiv = document.createElement("div")

        this.contentDiv.appendChild(panelPropertyDiv)

        // TODO: Avoid duplication with Inspector
        for(let childProperty of property.config.children){
            let divGenerator = GetPropertyDivGenerator(childProperty.type)
            let propertyDesc = divGenerator.generatePropertyDesc(childProperty)

            let propertyDiv = GenerateDiv(divGenerator, propertyDesc)

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

class ComponentPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new PanelPropertyDesc(property);
    }

    flexDirection(): string {
        return "column"
    }
}

let panelPropertyDivGenerator = new PanelPropertyDivGenerator()
let componentPropertyDivGenerator = new ComponentPropertyDivGenerator()
export {panelPropertyDivGenerator, componentPropertyDivGenerator}