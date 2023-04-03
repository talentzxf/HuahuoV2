import {AbstractGraphAction, ActionDef, ActionParam, GraphAction} from "./GraphActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

class BaseShapeActions extends AbstractGraphAction{
    targetShape:BaseShapeJS

    rotation = 0
    isRotationValid:boolean = false

    constructor(targetShape: BaseShapeJS) {
        super();

        this.targetShape = targetShape
    }

    @GraphAction()
    resetRotation(){
        this.rotation = 0.0
        this.isRotationValid = false
    }

    @GraphAction()
    rotateShape(@ActionParam(PropertyType.NUMBER) degree){
        this.rotation += degree

        this.isRotationValid = true
    }
}

export {BaseShapeActions }