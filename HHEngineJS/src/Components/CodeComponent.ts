import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import sha256 from 'crypto-js/sha256';
import {StringProperty} from "hhcommoncomponents";
import {renderEngine2D} from "../RenderEngine/RenderEnginePaperImpl";
import {CanvasEventEmitter} from "../RenderEngine/CanvasEventEmitter";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseShapeActor} from "../EventGraph/BaseShapeActor";

class UserDefinedClass {
    constructor(shape: BaseShapeActor, eventRegisterFunctions: object) {
    }

    onStart(){

    }

    onUpdate(){

    }
}

type UserDefinedConstructor = new (shape: BaseShapeActor, eventRegisterFunctions: object) => UserDefinedClass

@Component({compatibleShapes: ["BaseShapeJS"]})
class CodeComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", {textArea: true} as StringProperty)
    sourceCode

    userClassFunction: UserDefinedConstructor
    userClassObject: UserDefinedClass
    codeChecksum: string = ""

    startExecuted = false

    events = {}
    registerCanvasEvent(eventName, callback){
        let eventId = renderEngine2D.getEventBus().addEventHandler(CanvasEventEmitter.getEventEmitterName(), eventName, (param)=>{
            callback(param)
        })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let compileOK = true
        try{
            let checkSum = sha256(this.sourceCode)
            if(this.userClassFunction == null || this.codeChecksum != checkSum) {
                this.userClassFunction = (new Function(this.sourceCode))() as UserDefinedConstructor

                this.userClassObject = new this.userClassFunction(this.baseShape.getActor(), {
                    registerCanvasEvent: this.registerCanvasEvent.bind(this)
                })

                this.userClassObject.onStart()
                this.codeChecksum = checkSum
            }
        }catch (e){
            console.log("Compile error:", e)
            compileOK = false
        }

        if(compileOK){
            this.userClassObject.onUpdate()
        }
    }

    reset() {
        super.reset();
    }
}

export {CodeComponent}