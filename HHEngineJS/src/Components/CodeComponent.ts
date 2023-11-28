import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import sha256 from 'crypto-js/sha256';
import {renderEngine2D} from "../RenderEngine/RenderEnginePaperImpl";
import {CanvasEventEmitter} from "../RenderEngine/CanvasEventEmitter";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseShapeActor} from "../EventGraph/BaseShapeActor";


// Example:
// class Handler{
//     construct(shapeActor, eventRegisters){
//         this.shapeActor = shapeActor
//         this.eventRegisters = eventRegisters
//
//         console.log("HiHi")
//     }
//
//     onStart(){
//         console.log("onStart")
//     }
//
//     onUpdate(){
//         console.log("onUpdate")
//     }
// }

class UserDefinedClass {
    constructor(shape: BaseShapeActor, eventRegisterFunctions: object) {
    }

    onStart() {

    }

    onUpdate() {

    }
}

type UserDefinedConstructor = new (shape: BaseShapeActor, eventRegisterFunctions: object) => UserDefinedClass

@Component({compatibleShapes: ["BaseShapeJS"]})
class CodeComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", /*{textArea: true} as StringProperty*/ null, true)
    sourceCode

    @PropertyValue(PropertyCategory.customField)
    editSourceCode

    userClassObject: UserDefinedClass
    codeChecksum: string = ""

    startExecuted = false

    events = {}

    registerCanvasEvent(eventName, callback) {
        let eventId = renderEngine2D.getEventBus().addEventHandler(CanvasEventEmitter.getEventEmitterName(), eventName, (param) => {
            callback(param)
        })
    }

    isChecksumEqual(checksum1, checksum2) {
        if (!checksum1.hasOwnProperty("sigBytes") || !checksum1.hasOwnProperty("words")
            || !checksum2.hasOwnProperty("sigBytes") || !checksum1.hasOwnProperty("words")) {
            return false
        }

        if (!(checksum1["words"] instanceof Array) || !(checksum2["words"] instanceof Array))
            return false

        if (checksum1["words"].length != checksum2["words"].length)
            return false

        let word1 = checksum1["words"]
        let word2 = checksum2["words"]
        for (let idx = 0; idx < word1.length; idx++) {
            if (word1[idx] != word2[idx])
                return false
        }

        return true
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(this.sourceCode == null || this.sourceCode.length == 0)
            return

        let compileOK = true
        try {
            let checkSum = sha256(this.sourceCode)
            if (this.userClassObject == null || !this.isChecksumEqual(this.codeChecksum, checkSum)) {
                let classConstructor = null

                function RegisterClass(clz) {
                    classConstructor = clz
                }

                function GetUserClass() {
                    return classConstructor
                }

                (new Function('RegisterClass', this.sourceCode))(RegisterClass)
                let userClassConstructor = GetUserClass()
                this.userClassObject = new userClassConstructor(this.baseShape.getActor(), {
                    registerCanvasEvent: this.registerCanvasEvent.bind(this)
                })

                this.userClassObject.onStart()
                this.codeChecksum = checkSum
            }
        } catch (e) {
            console.log("Compile error:", e)
            compileOK = false
        }

        if (compileOK) {
            this.userClassObject.onUpdate()
        }
    }

    reset() {
        super.reset();
    }
}

export {CodeComponent}