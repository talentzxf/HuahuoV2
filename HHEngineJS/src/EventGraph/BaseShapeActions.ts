import {AbstractGraphAction, ActionDef, GraphAction} from "./GraphActionManager";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

class BaseShapeActions extends AbstractGraphAction{
    targetShape
    constructor(targetShape) {
        super();

        this.targetShape = targetShape
    }

    getActionDefs(): ActionDef[] {
        return [];
    }

    @GraphAction()
    rotateShape(@GraphParam(PropertyType.NUMBER) degree){
        this.targetShape.rotateAroundPivot(degree)
    }

}