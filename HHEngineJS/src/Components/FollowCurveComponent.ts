import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {ShapeArrayProperty} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {FloatPropertyConfig} from "hhcommoncomponents/dist/src/Properties/PropertyConfig";
import {huahuoEngine} from "../EngineAPI";

@Component({compatibleShapes: ["BaseShapeJS"], maxCount: 1})
class FollowCurveComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shape, null, {allowDuplication: false} as ShapeArrayProperty)
    targetShape

    // From 0.0 - 100.0
    // 0.0 -- At the beginning
    // 1.0 -- At the end.
    @PropertyValue(PropertyCategory.interpolateFloat, null, {max: 1.0, min: 0.0} as FloatPropertyConfig)
    lengthRatio: number = 0.0

    private tsFollowingTargetShape: BaseShapeJS

    private valueChangeHandlerId: number = -1;

    constructor(rawObj?) {
        super(rawObj);

        this.valueChangeHandler.registerValueChangeHandler("targetShape")(this.onTargetShapeChanged.bind(this))
    }

    onTargetShapeChanged(followShape: BaseShapeJS){
        if(this.tsFollowingTargetShape == followShape)
            return

        if(this.tsFollowingTargetShape && this.valueChangeHandlerId > 0){
            this.tsFollowingTargetShape.unregisterValueChangeHandler("*")(this.valueChangeHandlerId)
        }

        this.tsFollowingTargetShape = followShape

        if(followShape != null){
            this.baseShape.getAction().AddActionInvoker(this)

            let shapePosition = this.tsFollowingTargetShape.pivotPosition
            this.baseShape.getAction().setPosition(shapePosition.x, shapePosition.y)

            this.tsFollowingTargetShape.registerValueChangeHandler("*")(()=>{
                this.afterUpdate(true)
            })
        } else {
            this.baseShape.getAction().RemoveActionInvoker(this)

            this.valueChangeHandler.callHandlers("targetShape", null)
        }
    }

    getFollowingTargetShape(){
        if(this.tsFollowingTargetShape && !this.tsFollowingTargetShape.isValid()){
            this.targetShape = null
        }
        return this.tsFollowingTargetShape
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

                this.baseShape.getAction().setPosition(globalCurvePoint.x, globalCurvePoint.y)
                this.baseShape.afterUpdate(force) // Refresh the shape to reflect the change.
            }
        }
    }

}

export {FollowCurveComponent}