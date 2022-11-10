import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {FloatPropertyConfig} from "hhcommoncomponents";
import {CurveShapeJS} from "../Shapes/CurveShapeJS";

@Component({compatibleShapes: ["CurveShapeJS"]})
class CurveGrowthComponent extends AbstractComponent {

    @PropertyValue(PropertyCategory.interpolateFloat, 1.0, {min: 0.0, max: 1.0, step: 0.01} as FloatPropertyConfig)
    growth: number;

    clonedPaperShape

    get paperShape() {
        return this.baseShape.paperShape
    }

    setSegmentProperty(idx, property, value) {
        if (this.clonedPaperShape) {
            this.clonedPaperShape.segments[idx].point = this.baseShape.paperShape.segments[idx].point
            this.clonedPaperShape.segments[idx].handleIn = this.baseShape.paperShape.segments[idx].handleIn
            this.clonedPaperShape.segments[idx].handleOut = this.baseShape.paperShape.segments[idx].handleOut

            let updatedLength = this.paperShape.length / this.clonedPaperShape.length

            this.growth = updatedLength
        }
    }

    getSegments() {
        if (!this.clonedPaperShape) {
            return this.paperShape.segments
        }
        return this.clonedPaperShape.segments
    }

    // TODO, avoid duplication. This logic is the same as what we have in the BaseShapeJS.insertSegment.
    insertSegment(localPos) {
        if (this.clonedPaperShape) {
            let nearestPoint = this.clonedPaperShape.getNearestPoint(localPos)
            let offset = this.clonedPaperShape.getOffsetOf(nearestPoint)

            while (!this.clonedPaperShape.divideAt(offset)) {
                offset += 0.01 // Hit the corner points, offset a little and divide again.
            }
        }
    }

    override afterUpdate() {
        super.afterUpdate();

        let growth = this.growth
        if (growth < 1.0) {
            this.clonedPaperShape = this.paperShape.clone()
            this.clonedPaperShape.selected = false
            this.clonedPaperShape.visible = false
            this.clonedPaperShape.data = null

            let path2 = this.paperShape.splitAt(this.paperShape.length * growth)
            if (path2) {
                path2.visible = false
                path2.selected = false
                path2.remove()
            }
        }
    }
}

export {CurveGrowthComponent}