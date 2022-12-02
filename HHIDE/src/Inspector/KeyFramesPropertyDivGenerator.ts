import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {HHIntArray} from "./InputComponents/HHIntArray";
import {huahuoEngine} from "hhenginejs";
declare var Module:any

class KeyFramesPropertyDiv extends BasePropertyDesc{
    intArray: HHIntArray

    onValueChanged(val) {
    }

    constructor(property) {
        super(property);

        this.intArray = new HHIntArray(property.getter, property.setter, property.updater, property.deleter, this.titleDiv)
        this.contentDiv.appendChild(this.intArray)

        let keyFrameChangedHandler = new Module.ScriptEventHandlerImpl()
        keyFrameChangedHandler.handleEvent = this.onKeyFrameChanged.bind(this)
        huahuoEngine.GetInstance().RegisterEvent("OnKeyFrameChanged", keyFrameChangedHandler)
    }

    onKeyFrameChanged(){
        this.intArray.refresh()
    }
}

class KeyFramesPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new KeyFramesPropertyDiv(property);
    }

    flexDirection(): string {
        return "column"
    }
}

let keyFramesPropertyDivGenerator = new KeyFramesPropertyDivGenerator()
export {keyFramesPropertyDivGenerator}