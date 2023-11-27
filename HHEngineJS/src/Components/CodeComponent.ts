import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import sha256 from 'crypto-js/sha256';
import {StringProperty} from "hhcommoncomponents";

@Component({compatibleShapes: ["BaseShapeJS"]})
class CodeComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.stringValue, "", {textArea: true} as StringProperty)
    sourceCode

    userFunction: Function = null
    functionChecksum: string = ""

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let compileOK = true
        try{
            let checkSum = sha256(this.sourceCode)
            if(this.userFunction == null || this.functionChecksum != checkSum) {
                this.userFunction = new Function(this.sourceCode)
                this.functionChecksum = checkSum
            }
        }catch (e){
            console.log("Compile error:", e)
            compileOK = false
        }

        if(compileOK){
            this.userFunction()
        }

    }

    reset() {
        super.reset();
    }
}

export {CodeComponent}