import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

@Component({maxCount:1})
class FillColorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.interpolateColor, {random: true})
    fillColor

    override async afterUpdate(force: boolean = false) {
        await super.afterUpdate(force);

        this.baseShape.paperShape.fillColor = this.fillColor
    }
}

export {FillColorComponent}