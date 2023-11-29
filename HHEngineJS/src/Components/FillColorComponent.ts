import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";

@Component({maxCount:1})
class FillColorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.interpolateColor, {random: true})
    fillColor

    isBuiltIn = true

    canBeDisabled(): boolean {
        return false
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(this.baseShape.getActor().isFillColorValid){
            this.baseShape.paperShape.fillColor = this.baseShape.getActor().fillColor
        }else{
            this.baseShape.paperShape.fillColor = this.fillColor
        }

    }
}

export {FillColorComponent}