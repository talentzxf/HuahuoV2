import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {HHIntArray} from "./InputComponents/HHIntArray";
import {huahuoEngine, BaseShapeJS} from "hhenginejs";
declare var Module:any

class KeyFramesPropertyDiv extends BasePropertyDesc{
    intArray: HHIntArray

    targetObject: BaseShapeJS
    constructor(property) {
        super(property);

        this.targetObject = property.targetObject

        this.intArray = new HHIntArray(property.getter, property.inserter, property.updater, property.deleter, this.titleDiv)
        this.contentDiv.appendChild(this.intArray)

        let keyFrameChangedHandler = new Module.ScriptEventHandlerImpl()
        keyFrameChangedHandler.handleEvent = this.onKeyFrameChanged.bind(this)
        huahuoEngine.GetInstance().RegisterEvent("OnKeyFrameChanged", keyFrameChangedHandler)
    }

    onKeyFrameChanged(){
        if(huahuoEngine.isValidShape(this.targetObject)){
            this.intArray.refresh()
        }
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