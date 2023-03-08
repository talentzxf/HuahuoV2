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

    addEntry(componentEntry?): BasePropertyDesc {
        let subComponentTypeName = this.property.config.subComponentTypeName

        let subComponent = componentEntry
        if(subComponent == null){ // If not inputted the componentEntry, create a new one.
            subComponent = huahuoEngine.produceObject(subComponentTypeName)
            this.property.inserter(subComponent)
        }

        let propertyDivGenerator = GetPropertyDivGenerator(this.property.elementType)
        let propertyDesc = propertyDivGenerator.generatePropertyDesc(subComponent.getPropertySheet())
        let generatedDiv = GenerateDiv(propertyDivGenerator, propertyDesc)

        this.arrayEntryDivs.appendChild(generatedDiv)

        return propertyDesc;
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