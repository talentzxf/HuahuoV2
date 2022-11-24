import {BaseShapeJS} from "./BaseShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";

let shapeName = "NailShape"
class NailShapeJS extends BaseShapeJS{
    static createNail(rawObj){
        return new NailShapeJS(rawObj)
    }

    getShapeName(): string {
        return shapeName
    }
}

clzObjectFactory.RegisterClass(shapeName, NailShapeJS.createNail)
export {NailShapeJS}