import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {FloatPropertyConfig, GetObjPtr, IsValidWrappedObject} from "hhcommoncomponents";
import {LoadShapeFromCppShape} from "../Shapes/LoadShape";

//TODO: Move all these non default components into another sub-project
@Component()
class GeneratorComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    @PropertyValue(PropertyCategory.interpolateFloat, 0.1, {min: 0.01, max: 1.0, step: 0.01} as FloatPropertyConfig)
    generateInterval

    // BaseShape ObjPtr -> Mirages array.
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

        let usedShapes = new Set()

        for (let targetShape of this.targetShapeArray) {
            if (targetShape == null || !IsValidWrappedObject(targetShape.rawObj)) // The shape might not be loaded yet. But in next cycle, it should have been loaded.
                continue

            usedShapes.add(GetObjPtr(targetShape))

            let mirageShapeArray = this.targetShapeGeneratedShapeArrayMap.get(GetObjPtr(targetShape))
            if (mirageShapeArray == null) {
                mirageShapeArray = new Array<any>()
                this.targetShapeGeneratedShapeArrayMap.set(GetObjPtr(targetShape), mirageShapeArray)
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
                duplicatedShape.getActor().setPosition(position.x, position.y)

                duplicatedShape.update(force)
                index++
            }

            // Remove all other shapes.
            while (mirageShapeArray.length > index) {
                let tobeDeletedShape = mirageShapeArray.pop()
                tobeDeletedShape.removePaperObj()
            }
        }

        for (let [targetShapePtr, shapeArray] of this.targetShapeGeneratedShapeArrayMap) {
            if (!usedShapes.has(targetShapePtr)) {
                for (let miragetShape of shapeArray) {
                    miragetShape.removePaperObj()
                }

                this.targetShapeGeneratedShapeArrayMap.delete(targetShapePtr)
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