import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {ShapeArrayProperty} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

@Component({compatibleShapes: ["BaseShapeJS"], maxCount: 1})
class FollowCurveComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shape, null, {allowDuplication: false} as ShapeArrayProperty)
    targetShape: BaseShapeJS

    // From 0.0 - 100.0
    // 0.0 -- At the beginning
    // 100.0 -- At the end.
    @PropertyValue(PropertyCategory.interpolateFloat)
    lengthPercentage: number = 0.0

    constructor(rawObj?) {
        super(rawObj);

        this.valueChangeHandler.registerValueChangeHandler("targetShape")(this.onTargetShapeChanged.bind(this))
    }

    onTargetShapeChanged(followShape: BaseShapeJS){
        let shapePosition = followShape.pivotPosition
        this.baseShape.getAction().setPosition(shapePosition.x, shapePosition.y)
    }

}

export {FollowCurveComponent}