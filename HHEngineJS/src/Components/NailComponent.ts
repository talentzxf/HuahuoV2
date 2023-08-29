import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {NailShapeJS} from "../Shapes/NailShapeJS";
import {PropertyCategory} from "./PropertySheetBuilder";
import {ShapeArrayProperty} from "hhcommoncomponents";

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1, canBeFound: false})
class NailComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shapeArray, null, {allowDuplication: false} as ShapeArrayProperty) // It should actually be NailShape array. But as NailShape is actually also BaseShape, so it's fine.
    nails

    // isUpdating = false
    //
    // lastPosition = null
    // lastRotation = null

    // TODO: Check duplication! What if the nail has already been inserted??
    addNail(nail: NailShapeJS) {
        this["insertNails"](nail)
    }

    // rotationOrPositionChanged() {
    //     if (this.lastPosition == null || this.lastRotation == null)
    //         return false
    //
    //     if (this.baseShape.position.getDistance(this.lastPosition) > eps)
    //         return true
    //
    //     if (Math.abs(this.baseShape.rotation - this.lastRotation) > eps)
    //         return true
    //
    //     return false
    // }

    // afterUpdate(force: boolean = false) {
    //     if (this.isUpdating)
    //         return
    //
    //     this.isUpdating = true
    //     super.afterUpdate(force);
    //
    //     // if (this.rotationOrPositionChanged()) {
    //     //     getNailManager().shapeMoved(this.baseShape)
    //     // }
    //
    //     this.lastRotation = this.baseShape.rotation
    //     this.lastPosition = this.baseShape.position
    //
    //     this.isUpdating = false
    // }
}

export {NailComponent}