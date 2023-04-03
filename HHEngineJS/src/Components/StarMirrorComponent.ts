import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {createDuplication} from "./MirrorComponent";
import * as paper from "paper";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {FloatPropertyConfig} from "hhcommoncomponents";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";

class ClonedShapeEntry{
    shape: BaseShapeJS
    center: paper.Point
    angle: number
}

@Component({compatibleShapes:["StarMirrorShapeJS"], maxCount:1})
class StarMirrorComponent extends AbstractComponent{

    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    @PropertyValue(PropertyCategory.interpolateFloat, 120.0, {min: 1.0, max: 359.0, step: 1.0} as FloatPropertyConfig)
    starMirrorInterval

    targetShapeMirroredShapeSetMap: Map<number, Set<BaseShapeJS>> = new Map<number, Set<BaseShapeJS>>()
    mirroredShapeShapeEntryMap: Map<BaseShapeJS, ClonedShapeEntry> = new Map

    paperShapeGroup: paper.Group

    constructor(rawObj?) {
        super(rawObj);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape
    }

    getMirroredShapeSet(rawPtr: number):Set<BaseShapeJS>{
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
            let centerPosition = this.baseShape.position

            this.getMirroredShapeSet(targetShape.rawObj.ptr).add(duplicatedShape)

            this.mirroredShapeShapeEntryMap.set(duplicatedShape, {
                shape: duplicatedShape,
                center: centerPosition,
                angle: currentAngle
            })

            this.paperShapeGroup.addChild(duplicatedShape.paperItem)
            let _this = this
            duplicatedShape.getParent = function(){
                return _this.baseShape.getParent()
            }

            targetShape.registerValueChangeHandler("*")(()=>{
                duplicatedShape.update(true)
            })

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

                        for(let mirroredShape of this.getMirroredShapeSet(targetShape.rawObj.ptr)){

                        }
                    }
                }
            }
        }
    }
}

export {StarMirrorComponent}