import {AbstractGraphAction, ActionDef, ActionParam, GraphAction} from "./GraphActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"
import {Vector2} from "hhcommoncomponents"
import {AbstractComponent} from "../Components/AbstractComponent";

class BaseShapeActor extends AbstractGraphAction{
    targetShape:BaseShapeJS

    position: Vector2 = new Vector2()
    isPositionValid:boolean = false

    rotation = 0
    isRotationValid:boolean = false

    actionInvokers: Set<AbstractComponent> = new Set<AbstractComponent>()

    constructor(targetShape: BaseShapeJS) {
        super();

        this.targetShape = targetShape
    }

    AddActionInvoker(component: AbstractComponent){
        this.actionInvokers.add(component)
    }

    RemoveActionInvoker(component: AbstractComponent){
        this.actionInvokers.delete(component)

        if(this.actionInvokers.size == 0){
            this.reset()
        }
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
        if(isNaN(x) || isNaN(y))
            return

        this.position.x = x
        this.position.y = y

        this.isPositionValid = true
    }

    @GraphAction()
    move(@ActionParam(PropertyType.VECTOR2) dir: paper.Point, @ActionParam(PropertyType.NUMBER) speed: number = 1.0){
        if(dir == null || isNaN(dir.x) || isNaN(dir.y))
            return

        this.position = this.position.add(dir.multiply(speed))

        this.isPositionValid = true
    }

    @GraphAction()
    rotateShape(@ActionParam(PropertyType.NUMBER) degree){
        if(isNaN(degree))
            return

        this.rotation += degree

        this.isRotationValid = true
    }

    @GraphAction()
    lookAt(@ActionParam(PropertyType.VECTOR2) targetPoint){
        if(targetPoint == null || isNaN(targetPoint.x) || isNaN(targetPoint.y)){
            return
        }

        let curShapePosition = this.targetShape.position
        let degree = targetPoint.subtract(curShapePosition).angle

        this.rotation = degree
        this.isRotationValid = true

    }
}

export {BaseShapeActor }