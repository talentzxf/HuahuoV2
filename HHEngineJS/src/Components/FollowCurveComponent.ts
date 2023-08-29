import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {ShapeArrayProperty} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {huahuoEngine} from "../EngineAPI";
import {IsValidWrappedObject, FloatPropertyConfig} from "hhcommoncomponents";

@Component({compatibleShapes: ["BaseShapeJS"], maxCount: 1})
class FollowCurveComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shape, null, {allowDuplication: false} as ShapeArrayProperty)
    targetShape: BaseShapeJS

    // From 0.0 - 100.0
    // 0.0 -- At the beginning
    // 1.0 -- At the end.
    @PropertyValue(PropertyCategory.interpolateFloat, null, {max: 1.0, min: 0.0} as FloatPropertyConfig)
    lengthRatio: number = 0.0

    private valueChangeHandlerId: number = -1;

    constructor(rawObj?) {
        super(rawObj);

        this.valueChangeHandler.registerValueChangeHandler("targetShape")(this.onTargetShapeChanged.bind(this))
    }

    onTargetShapeChanged(followShape: BaseShapeJS){
        if(this.targetShape.getRawObject().ptr == followShape.getRawObject().ptr)
            return

        if(this.targetShape && this.valueChangeHandlerId > 0){
            this.targetShape.unregisterValueChangeHandler("*")(this.valueChangeHandlerId)
        }

        this.targetShape = followShape

        if(followShape != null){
            this.baseShape.getActor().AddActionInvoker(this)

            let shapePosition = this.targetShape.pivotPosition
            this.baseShape.getActor().setPosition(shapePosition.x, shapePosition.y)

            this.targetShape.registerValueChangeHandler("*")(()=>{
                this.afterUpdate(true)
            })
        } else {
            this.baseShape.getActor().RemoveActionInvoker(this)

            this.valueChangeHandler.callHandlers("targetShape", null)
        }
    }

    getFollowingTargetShape(){
        return this.targetShape
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(!this.isComponentActive()){
            return
        }

        if(this.baseShape.isVisible()){
            let followTargetShape = this.getFollowingTargetShape()
            if(followTargetShape){
                followTargetShape.afterUpdate(true) // Force update the curveShape.

                let totalLength = followTargetShape.length()
                let targetLength = totalLength * this.lengthRatio
                let curvePoint = followTargetShape.getPointAt(targetLength)

                let globalCurvePoint = followTargetShape.localToGlobal(curvePoint)

                this.baseShape.getActor().setPosition(globalCurvePoint.x, globalCurvePoint.y)
                this.baseShape.afterUpdate(force) // Refresh the shape to reflect the change.
            }
        }
    }

}

export {FollowCurveComponent}