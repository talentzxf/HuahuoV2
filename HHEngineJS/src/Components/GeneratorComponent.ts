import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {FloatPropertyConfig} from "hhcommoncomponents";
import {LoadShapeFromCppShape} from "../Shapes/LoadShape";

//TODO: Move all these non default components into another sub-project
@Component()
class GeneratorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    @PropertyValue(PropertyCategory.interpolateFloat, 0.1, {min:0.01, max:1.0, step: 0.01} as FloatPropertyConfig)
    generateInterval

    targetShapeGeneratedShapeArrayMap: Map<BaseShapeJS, Array<any>> = new Map<BaseShapeJS, Array<any>>()

    constructor(rawObj?) {
        super(rawObj);
    }

    afterUpdate() {
        super.afterUpdate();

        for(let targetShape of this.targetShapeArray){
            let baseShapeJS = this.baseShape.paperShape
            if(!this.targetShapeGeneratedShapeArrayMap.has(targetShape)){
                let newGeneratedShapeArray = new Array<any>()
                this.targetShapeGeneratedShapeArrayMap.set(targetShape, newGeneratedShapeArray)

                // Duplicate shapes along the edge.
                for(let currentLengthRatio = 0.0; currentLengthRatio < 1.0; currentLengthRatio += this.generateInterval){
                    let currentLength = baseShapeJS.length * currentLengthRatio
                    let rawObj = targetShape.rawObj
                    let duplicatedShape = LoadShapeFromCppShape(rawObj)
                    duplicatedShape.setSelectedMeta(null)

                    let position = baseShapeJS.localToGlobal(baseShapeJS.getPointAt(currentLength))

                    duplicatedShape.position = position
                }
            }
        }
    }
}

export {GeneratorComponent}