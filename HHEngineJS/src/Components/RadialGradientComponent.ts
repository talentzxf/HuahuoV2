import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

@Component({compatibleShapes:["BaseSolidShape"], maxCount: 1})
class RadialGradientComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.colorStopArray)
    gradientColorArray

    @PropertyValue(PropertyCategory.interpolateVector2, {x: 0.0, y: 0.0})
    centerPosition

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        // TODO: Sort first??
        let stops = []
        for(let colorStop of this.gradientColorArray){
            let color = new paper.Color(colorStop.r, colorStop.g, colorStop.b, colorStop.a)
            let value = colorStop.value

            stops.push([color, value])
        }

        let _this = this
        this.baseShape.paperShape.fillColor = new paper.Color({
            gradient:{
                stops: stops,
                radial: true
            },
            origin: _this.baseShape.paperShape.position.add(this.centerPosition),
            destination: _this.baseShape.bounds.rightCenter
        })
    }
}

export {RadialGradientComponent}