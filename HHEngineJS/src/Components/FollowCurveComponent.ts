import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

@Component({compatibleShapes: ["BaseShapeJS"], maxCount: 1})
class FollowCurveComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shape)
    targetShape

}

export {FollowCurveComponent}