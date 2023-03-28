import {AbstractGraphAction, ActionDef, ActionParam, GraphAction} from "./GraphActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

class BaseShapeActions extends AbstractGraphAction{
    targetShape:BaseShapeJS

    rotation = 0

    constructor(targetShape: BaseShapeJS) {
        super();

        this.targetShape = targetShape
    }

    @GraphAction()
    rotateShape(@ActionParam(PropertyType.NUMBER) degree){
        this.rotation = degree
    }

    execute(){
        this.targetShape.rotateAroundPivot(this.rotation, false)
    }
}

export {BaseShapeActions }