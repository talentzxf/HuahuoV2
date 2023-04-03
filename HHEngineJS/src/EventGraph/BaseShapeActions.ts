import {AbstractGraphAction, ActionDef, ActionParam, GraphAction} from "./GraphActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"
import {Vector2} from "hhcommoncomponents"

class BaseShapeActions extends AbstractGraphAction{
    targetShape:BaseShapeJS

    position: Vector2 = new Vector2()
    isPositionValid:boolean = false

    rotation = 0
    isRotationValid:boolean = false

    constructor(targetShape: BaseShapeJS) {
        super();

        this.targetShape = targetShape
    }

    @GraphAction()
    reset(){
        this.rotation = 0.0
        this.position = new Vector2()
        this.isRotationValid = false
        this.isPositionValid = false
    }

    @GraphAction()
    setPosition(@ActionParam(PropertyType.NUMBER) x:number, @ActionParam(PropertyType.NUMBER) y:number){
        this.position.x = x
        this.position.y = y

        this.isPositionValid = true
    }

    @GraphAction()
    rotateShape(@ActionParam(PropertyType.NUMBER) degree){
        this.rotation += degree

        this.isRotationValid = true
    }
}

export {BaseShapeActions }