import {PropertySheetBuilder} from "./PropertySheetBuilder";
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyType} from "hhcommoncomponents";
import {Property} from "hhcommoncomponents";

class ShapeArrayPropertySheetBuilder extends PropertySheetBuilder{
    override build(component, propertyMeta, valueChangeHandler: ValueChangeHandler){
        let propertySheet = super.build(component, propertyMeta, valueChangeHandler);
        propertySheet["type"] = PropertyType.ARRAY
        propertySheet["elementType"] = Property.REFERENCE

        return propertySheet
    }
}

export {ShapeArrayPropertySheetBuilder}