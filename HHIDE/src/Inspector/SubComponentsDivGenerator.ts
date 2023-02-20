import {
    BasePropertyDesc,
    BasePropertyDivGenerator,
    GenerateDiv,
    GetPropertyDivGenerator
} from "./BasePropertyDivGenerator";
import {ArrayPropertyDesc} from "./ArrayPropertyDivGenerator";
import {huahuoEngine} from "hhenginejs";

class SubComponentsPropertyDesc extends ArrayPropertyDesc {
    constructor(property) {
        super(property);
    }

    onValueChanged(val) {
    }

    addEntry(): BasePropertyDesc {
        let subComponentTypeName = this.property.config.subComponentTypeName

        let subComponent = huahuoEngine.produceObject(subComponentTypeName)
        this.property.inserter(subComponent)

        let propertyDivGenerator = GetPropertyDivGenerator(this.property.elementType)
        let propertyDesc = propertyDivGenerator.generatePropertyDesc(subComponent.getPropertySheet())
        let generatedDiv = GenerateDiv(propertyDivGenerator, propertyDesc)

        this.arrayEntryDivs.appendChild(generatedDiv)

        return null;
    }
}

class SubComponentsDivGenerator extends BasePropertyDivGenerator {
    generatePropertyDesc(property): BasePropertyDesc {
        let subcomponentPropertyDesc = new SubComponentsPropertyDesc(property)
        return subcomponentPropertyDesc;
    }
}

let subComponentsDivGenerator = new SubComponentsDivGenerator()

export {subComponentsDivGenerator}