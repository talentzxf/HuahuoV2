import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {ArrayPropertyDesc} from "./ArrayPropertyDivGenerator";

class SubComponentsPropertyDesc extends ArrayPropertyDesc {
    constructor(property) {
        super(property);
    }

    onValueChanged(val) {
    }

    addEntry(): BasePropertyDesc {
        return super.addEntry();
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