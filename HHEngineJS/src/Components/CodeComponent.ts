import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import sha256 from 'crypto-js/sha256';
import {renderEngine2D} from "../RenderEngine/RenderEnginePaperImpl";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseShapeActor} from "../EventGraph/BaseShapeActor";
import {HHEventBus} from "hhcommoncomponents";
import {UserDefinedComponent} from "./UserDefinedComponent";

type UserDefinedConstructor = new (shape: BaseShapeActor, eventRegisterFunctions: object) => UserDefinedComponent

@Component({compatibleShapes: ["BaseShapeJS"]})
class CodeComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", /*{textArea: true} as StringProperty*/ null, true)
    sourceCode

    @PropertyValue(PropertyCategory.customField)
    editSourceCode

    userClassObject: UserDefinedComponent
    codeChecksum: string = ""

    startExecuted = false

    events = {}

    eventMap: Map<HHEventBus, Set<number>> = new Map()

    getEventSet(eventBus: HHEventBus) {
        if (!this.eventMap.has(eventBus)) {
            this.eventMap.set(eventBus, new Set())
        }

        return this.eventMap.get(eventBus)
    }

    registerCanvasEvent(eventName, callback) {
        let canvasEventBus = renderEngine2D.getEventBus()
        let eventId = canvasEventBus.addEventHandler(renderEngine2D.getEventEmitterName(), eventName, (param) => {
            callback(param)
        })

        this.getEventSet(renderEngine2D.getEventBus()).add(eventId)
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

    onStartExecuted = false

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (this.sourceCode == null || this.sourceCode.length == 0)
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

                this.codeChecksum = checkSum
            }
        } catch (e) {
            console.log("Compile error:", e)
            compileOK = false
        }

        if (compileOK) {
            if(!this.onStartExecuted){
                if (this.userClassObject.onStart){
                    this.userClassObject.onStart()

                    this.onStartExecuted = true
                }
            }

            if (this.userClassObject.onUpdate)
                this.userClassObject.onUpdate()
        }
    }

    reset() {
        super.reset();
        this.onStartExecuted = false

        for(let [eventBus, eventSet] of this.eventMap){
            for(let handlerId of eventSet){
                eventBus.removeEventHandlerFromAllEvents(handlerId)
            }
        }

        if(this.userClassObject){
            this.userClassObject.onReset()
        }
    }
}

export {CodeComponent}