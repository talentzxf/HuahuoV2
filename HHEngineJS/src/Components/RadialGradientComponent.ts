import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {StringProperty} from "hhcommoncomponents";

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1})
class RadialGradientComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.colorStopArray)
    gradientColorArray

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 0.0, y: 0.0})
    centerPosition

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 0.0, y: 0.0})
    destinationPosition

    @PropertyValue(PropertyCategory.stringValue, "radial", {options: ["radial", "linear"]} as StringProperty)
    gradientType

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (this.isComponentActive()) {
            this.onComponentEnabled()
        } else {
            this.onComponentDisabled()
        }
    }

    onComponentEnabled() {
        // TODO: Sort first??
        let stops = []
        for (let colorStop of this.gradientColorArray) {
            let color = new paper.Color(colorStop.r, colorStop.g, colorStop.b, colorStop.a)
            let value = colorStop.value

            stops.push([color, value])
        }

        let _this = this
        let fillColorConfig = {
            gradient: {
                stops: stops,
                radial: true
            },
            origin: _this.baseShape.paperShape.position.add(this.centerPosition),
            destination: _this.baseShape.bounds.rightCenter.add(this.destinationPosition)
        }

        if (this.gradientType == "linear") {
            fillColorConfig.gradient.radial = false
        } else {
            fillColorConfig.gradient.radial = true
        }

        this.baseShape.getActor().setFillColor(new paper.Color(fillColorConfig))
    }

    onComponentDisabled() {
        this.baseShape.getActor().setFillColor(null)
    }
}

export {RadialGradientComponent}