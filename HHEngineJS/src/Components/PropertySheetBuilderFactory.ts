import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {InterpolatePropertyBuilder, PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
import {StaticPropertySheetBuilder} from "./StaticPropertyBuilder";

class PropertySheetFactory{
    interpolatePropertySheetBuilder = new InterpolatePropertyBuilder()
    staticPropertySheetBuilder = new StaticPropertySheetBuilder()
    getBuilder(category){
        switch (category){
            case PropertyCategory.interpolate:
                return this.interpolatePropertySheetBuilder
            case PropertyCategory.static:
                return this.staticPropertySheetBuilder
        }
    }

    createEntry(component, propertyMeta:PropertyDef, valueChangeHandler: ValueChangeHandler){
        let builder = this.getBuilder(propertyMeta.type)
        if(!builder)
            return
        return builder.build(component, propertyMeta, valueChangeHandler)
    }
}

let propertySheetFactory = new PropertySheetFactory()

export {propertySheetFactory}