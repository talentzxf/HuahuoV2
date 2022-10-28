import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {InterpolatePropertyBuilder, PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
import {ShapeArrayPropertySheetBuilder} from "./ShapeArrayPropertySheetBuilder";

class PropertySheetFactory{
    interpolatePropertySheetBuilder = new InterpolatePropertyBuilder()
    shapeArrayPropertySheetBuilder = new ShapeArrayPropertySheetBuilder()
    getBuilder(category){
        switch (category){
            case PropertyCategory.interpolate:
                return this.interpolatePropertySheetBuilder
            case PropertyCategory.shapeArray:
                return this.shapeArrayPropertySheetBuilder
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