import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {createDuplication} from "./MirrorComponent";
import * as paper from "paper";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";
import {FloatPropertyConfig} from "hhcommoncomponents/dist/src/Properties/PropertyConfig";

@Component({compatibleShapes:["StarMirrorShapeJS"], maxCount:1})
class StarMirrorComponent extends AbstractComponent{

    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    @PropertyValue(PropertyCategory.interpolateFloat, 120.0, {min: 1.0, max: 359.0, step: 1.0} as FloatPropertyConfig)
    starMirrorInterval

    targetShapeMirroredShapeSetMap: Map<number, Set<BaseShapeJS>> = new Map<number, Set<BaseShapeJS>>()

    paperShapeGroup: paper.Group

    constructor(rawObj?) {
        super(rawObj);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape
    }

    getMirroredShapeSet(rawPtr: number){
        if(!this.targetShapeMirroredShapeSetMap.has(rawPtr)){
            this.targetShapeMirroredShapeSetMap.set(rawPtr, new Set<BaseShapeJS>())
        }

        return this.targetShapeMirroredShapeSetMap.get(rawPtr)
    }

    duplicateShapes(targetShape){
        let currentAngle = this.starMirrorInterval

        let targetPosition = targetShape.position

        while(currentAngle < 360){
            let duplicatedShape = createDuplication(targetShape, this.baseShape)
            this.getMirroredShapeSet(targetShape.rawObj.ptr).add(duplicatedShape)

            this.paperShapeGroup.addChild(duplicatedShape.paperItem)
            let _this = this
            duplicatedShape.getParent = function(){
                return _this.baseShape.getParent()
            }

            targetShape.registerValueChangeHandler("*")(()=>{
                duplicatedShape.update(true)
            })

            let centerPosition = this.baseShape.position
            let newPosition = targetPosition.rotate(currentAngle, centerPosition)
            duplicatedShape.position = newPosition

            currentAngle += this.starMirrorInterval
        }

    }

    override afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(this.baseShape.isVisible()){
            let baseShapeParent = this.baseShape.paperShape.parent
            if(baseShapeParent != null && this.paperShapeGroup.parent != baseShapeParent)
                baseShapeParent.addChild(this.paperShapeGroup)

            if(this.targetShapeArray){
                for(let targetShape of this.targetShapeArray){
                    if(targetShape != null){
                        if(!this.targetShapeMirroredShapeSetMap.has(targetShape.rawObj.ptr)){
                            this.duplicateShapes(targetShape)
                        }
                    }
                }
            }
        }
    }
}

export {StarMirrorComponent}