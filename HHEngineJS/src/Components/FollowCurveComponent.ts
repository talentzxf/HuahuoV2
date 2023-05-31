import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {ShapeArrayProperty} from "hhcommoncomponents";

@Component({compatibleShapes: ["BaseShapeJS"], maxCount: 1})
class FollowCurveComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shape, null, {allowDuplication: false} as ShapeArrayProperty)
    targetShape

    constructor(rawObj?) {
        super(rawObj);



    }

}

export {FollowCurveComponent}