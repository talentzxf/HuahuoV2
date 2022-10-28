import {PropertySheetBuilder} from "./PropertySheetBuilder";
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";

class StaticPropertySheetBuilder extends PropertySheetBuilder{
    override build(component, propertyMeta, valueChangeHandler: ValueChangeHandler){
        return super.build(component, propertyMeta, valueChangeHandler);
    }
}

export {StaticPropertySheetBuilder}