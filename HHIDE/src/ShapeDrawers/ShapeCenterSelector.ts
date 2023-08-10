import {BaseShapeJS} from "hhenginejs/dist/src/Shapes/BaseShapeJS";
import {PropertySheet, PropertyType} from "HHCommonComponents";
import {ValueChangeHandler} from "hhenginejs";

class ShapeCenterSelector {
    private _targetObj: BaseShapeJS;
    private readonly propertySheet: PropertySheet
    private _isSelected: boolean = false;

    private circleShape: paper.Path

    private valueChangeHandler: ValueChangeHandler = new ValueChangeHandler()

    constructor(targetObj: BaseShapeJS) {
        this._targetObj = targetObj

        this.propertySheet = new PropertySheet();

        this.propertySheet.addProperty({
                key: "inspector.Type",
                type: PropertyType.STRING,
                getter: this.getTypeName.bind(this)
            },
            {
                key: "inspector.FixedPosition",
                type: PropertyType.VECTOR2,
                getter: this.getPosition.bind(this),
                setter: this.setPosition.bind(this),
                registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("centerPosition").bind(this),
                unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("centerPosition").bind(this)
            })
    }

    globalToLocal(globalPosition: paper.Point): paper.Point{
        return this.paperShape.globalToLocal(globalPosition)
    }

    localToGlobal(localPosition: paper.Point): paper.Point{
        return this.paperShape.localToGlobal(localPosition)
    }

    getTypeName() {
        return "CenterOf"
    }

    get name() {
        let retName = "[" + this._targetObj.getTypeName() + "]"
        if (this._targetObj.name)
            retName += this._targetObj.name
        return retName
    }

    getLayer() {
        return this._targetObj.getLayer()
    }

    get position() {
        return this.paperShape.position
    }

    set position(val: paper.Point) {
        this.paperShape.position = val

        this._targetObj.pivotPosition = val

        this.valueChangeHandler.callHandlers("centerPosition", val)
    }

    getPosition() {
        return this.position
    }

    setPosition(val: paper.Point) {
        if (this._isSelected) {
            this.position = this._targetObj.pivotPosition

            this.valueChangeHandler.callHandlers("centerPosition", val)
        }
    }

    get paperShape() {
        if (!this.circleShape) {
            let paperJs = this._targetObj.getPaperJs()
            this.circleShape = new paperJs.Path.Circle(this._targetObj.pivotPosition, 20)
            this.circleShape.fillColor = new paper.Color("red")
            this.circleShape.applyMatrix = false
        }
        return this.circleShape
    }

    isSelectable() {
        return this._targetObj.isLocked()
    }

    getParent() {
        return null
    }

    set selected(val: boolean) {
        if (!val) {
            if (this.circleShape) {
                this.circleShape.remove()
                this.circleShape = null
            }
        } else {
            this.paperShape
        }

        this._isSelected = val
    }

    update() {
    }

    getPropertySheet() {
        return this.propertySheet
    }

    store() {

    }

    isMovable(){
        return true
    }

    getBornStoreId() {
        return this._targetObj.getBornStoreId()
    }
}

export {ShapeCenterSelector}