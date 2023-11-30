import {AbstractGraphAction, ActionDef, ActionParam, GraphAction, ReturnValueInfo} from "./GraphActions";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"
import {Vector2} from "hhcommoncomponents"
import {AbstractComponent} from "../Components/AbstractComponent";

class BaseShapeActor extends AbstractGraphAction {
    targetShape: BaseShapeJS

    position: Vector2 = new Vector2()
    _isPositionValid: boolean = false

    rotation = 0
    _isRotationValid: boolean = false

    actionInvokers: Set<AbstractComponent> = new Set<AbstractComponent>()

    constructor(targetShape: BaseShapeJS) {
        super();

        this.targetShape = targetShape
    }

    get isRotationValid() {
        if (this.targetShape.isResetting)
            return false
        return this._isRotationValid
    }

    get isPositionValid() {
        if (this.targetShape.isResetting)
            return false
        return this._isPositionValid
    }

    getRawObject() {
        return this.targetShape.getRawObject()
    }

    AddActionInvoker(component: AbstractComponent) {
        this.actionInvokers.add(component)
    }

    RemoveActionInvoker(component: AbstractComponent) {
        this.actionInvokers.delete(component)

        if (this.actionInvokers.size == 0) {
            this.reset()
        }
    }

    @GraphAction(true, {
        valueName: "position",
        valueType: "vec2"
    } as ReturnValueInfo)
    GetPosition() {
        return this.targetShape.position
    }

    @GraphAction()
    reset() {
        this.rotation = 0.0
        this.position = new Vector2()
        this._isRotationValid = false
        this._isPositionValid = false
    }

    @GraphAction()
    setPosition(@ActionParam(PropertyType.NUMBER) x: number, @ActionParam(PropertyType.NUMBER) y: number) {
        if(this.targetShape.isResetting)
            return

        if (isNaN(x) || isNaN(y))
            return

        this.position.x = x
        this.position.y = y

        this._isPositionValid = true
    }

    @GraphAction()
    setRotation(@ActionParam(PropertyType.NUMBER) degree: number) {
        if(this.targetShape.isResetting)
            return

        if (isNaN(degree))
            return
        this.rotation = degree
        this._isRotationValid = true
    }

    @GraphAction()
    move(@ActionParam(PropertyType.VECTOR2) dir: paper.Point, @ActionParam(PropertyType.NUMBER) speed: number = 1.0) {
        if(this.targetShape.isResetting)
            return

        if (dir == null || isNaN(dir.x) || isNaN(dir.y))
            return

        if (!(dir instanceof paper.Point)) {
            dir = new paper.Point(dir)
        }

        this.position = this.targetShape.position.add(dir.multiply(speed))

        this._isPositionValid = true
    }

    @GraphAction()
    rotateShape(@ActionParam(PropertyType.NUMBER) degree) {
        if(this.targetShape.isResetting)
            return

        if (isNaN(degree))
            return

        this.rotation = this.targetShape.rotation + degree

        this._isRotationValid = true
    }

    @GraphAction()
    lookAt(@ActionParam(PropertyType.VECTOR2) targetPoint) {
        if(this.targetShape.isResetting)
            return

        if (targetPoint == null || isNaN(targetPoint.x) || isNaN(targetPoint.y)) {
            return
        }

        let curShapePosition = this.targetShape.position
        let degree = targetPoint.subtract(curShapePosition).angle

        this.rotation = degree
        this._isRotationValid = true
    }
}

export {BaseShapeActor}