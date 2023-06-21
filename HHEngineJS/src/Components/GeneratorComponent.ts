import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {FloatPropertyConfig} from "hhcommoncomponents";
import {LoadShapeFromCppShape} from "../Shapes/LoadShape";

//TODO: Move all these non default components into another sub-project
@Component()
class GeneratorComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    @PropertyValue(PropertyCategory.interpolateFloat, 0.1, {min: 0.01, max: 1.0, step: 0.01} as FloatPropertyConfig)
    generateInterval

    // BaseShape.rawObj.ptr -> Mirages array.
    targetShapeGeneratedShapeArrayMap: Map<number, Array<any>> = new Map<number, Array<any>>()

    paperShapeGroup: paper.Group

    constructor(rawObj?, isMirage = false) {
        super(rawObj, isMirage);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape
    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        this.paperShapeGroup.rotation = this.baseShape.rotation
        this.paperShapeGroup.scaling = this.baseShape.scaling

        for (let targetShape of this.targetShapeArray) {
            if (targetShape == null) // The shape might not be loaded yet. But in next cycle, it should have been loaded.
                continue

            let mirageShapeArray = this.targetShapeGeneratedShapeArrayMap.get(targetShape.rawObj.ptr)
            if (mirageShapeArray == null) {
                mirageShapeArray = new Array<any>()
                this.targetShapeGeneratedShapeArrayMap.set(targetShape.rawObj.ptr, mirageShapeArray)
            }

            let baseShapeJS = this.baseShape.paperShape
            let index = 0
            // Duplicate shapes along the edge.
            for (let currentLengthRatio = 0.0; currentLengthRatio <= this.baseShape.getMaxLengthRatio(); currentLengthRatio += this.generateInterval) {
                let duplicatedShape = null
                if (mirageShapeArray.length <= index) {
                    let rawObj = targetShape.rawObj
                    duplicatedShape = LoadShapeFromCppShape(rawObj, false, false, true)
                    duplicatedShape.update(true)

                    duplicatedShape.isSelectable = function () {
                        return false
                    }

                    duplicatedShape.setSelectedMeta(null)
                    duplicatedShape.isMirage = true
                    mirageShapeArray.push(duplicatedShape)

                    targetShape.registerValueChangeHandler("*")(() => {
                        duplicatedShape.update(true)
                    })

                    this.paperShapeGroup.addChild(duplicatedShape.paperShape)
                } else {
                    duplicatedShape = mirageShapeArray[index]
                }

                let currentLength = this.baseShape.getCurveLength() * currentLengthRatio
                let position = this.paperShapeGroup.globalToLocal(baseShapeJS.localToGlobal(baseShapeJS.getPointAt(currentLength)))
                duplicatedShape.getAction().setPosition(position.x, position.y)

                duplicatedShape.update(force)
                index++
            }

            // Remove all other shapes.
            while (mirageShapeArray.length > index) {
                let tobeDeletedShape = mirageShapeArray.pop()
                tobeDeletedShape.removePaperObj()
            }
        }
    }

    cleanUp() {
        super.cleanUp();

        for (let [targetShape, shapeArray] of this.targetShapeGeneratedShapeArrayMap) {
            for (let mirageShape of shapeArray) {
                mirageShape.removePaperObj()
            }
        }

        this.targetShapeGeneratedShapeArrayMap = new Map<number, Array<any>>()
    }
}

export {GeneratorComponent}