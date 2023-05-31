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

    tsTargetShape: BaseShapeJS

    constructor(rawObj?) {
        super(rawObj);

        this.valueChangeHandler.registerValueChangeHandler("targetShape")(this.onTargetShapeChanged.bind(this))
    }

    onTargetShapeChanged(followShape: BaseShapeJS){
        this.tsTargetShape = followShape

        let shapePosition = this.tsTargetShape.pivotPosition
        this.baseShape.getAction().setPosition(shapePosition.x, shapePosition.y)
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(this.baseShape.isVisible()){
            if(this.tsTargetShape && this.tsTargetShape){
                let totalLength = this.tsTargetShape.length()
                let targetLength = totalLength * this.lengthRatio
                let curvePoint = this.tsTargetShape.getPointAt(targetLength)

                this.baseShape.getAction().setPosition(curvePoint.x, curvePoint.y)
            }
        }
    }

}

export {FollowCurveComponent}