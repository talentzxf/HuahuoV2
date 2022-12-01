import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";

class KeyFramesPropertyDiv extends BasePropertyDesc{
    onValueChanged(val) {
    }

    constructor(property) {
        super(property);
    }

}

class KeyFramesPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new KeyFramesPropertyDiv(property);
    }

}

let keyFramesPropertyDivGenerator = new KeyFramesPropertyDivGenerator()
export {keyFramesPropertyDivGenerator}