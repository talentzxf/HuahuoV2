import {AbstractGraphAction, ActionDef, ActionParam, GraphAction} from "./GraphActionManager";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

class BaseShapeActions extends AbstractGraphAction{
    targetShape:BaseShapeJS
    constructor(targetShape: BaseShapeJS) {
        super();

        this.targetShape = targetShape
    }

    getActionDefs(): ActionDef[] {
        return [];
    }

    @GraphAction()
    rotateShape(@ActionParam(PropertyType.NUMBER) degree){
        this.targetShape.rotateAroundPivot(degree)
    }

}

export {BaseShapeActions }