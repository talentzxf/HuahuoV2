import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {createDuplication} from "./MirrorComponent";
import * as paper from "paper";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {FloatPropertyConfig, GetObjPtr} from "hhcommoncomponents";

class ClonedShapeEntry{
    shape: BaseShapeJS
    targetShape: BaseShapeJS
    centerObject: BaseShapeJS
    angle: number
    isUsing: boolean
}

function updateEntry(entry: ClonedShapeEntry){
    if(entry.isUsing){
        let parentGroup = entry.shape.paperShape.parent

        parentGroup.rotation = 0.0

        let pointZeroPosition = parentGroup.localToParent(new paper.Point(0,0))
        parentGroup.position.x -= pointZeroPosition.x
        parentGroup.position.y -= pointZeroPosition.y

        parentGroup.rotate(entry.angle, entry.centerObject.position)
        entry.shape.update()

        if(entry.targetShape && entry.targetShape.paperShape)
            entry.shape.paperShape.visible = entry.targetShape.paperShape.visible
    }else{
        entry.shape.hide()
    }

}

@Component({compatibleShapes:["StarMirrorShapeJS"], maxCount:1})
class StarMirrorComponent extends AbstractComponent{

    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    @PropertyValue(PropertyCategory.interpolateFloat, 100.0, {min: 1.0, max: 359.0, step: 1.0} as FloatPropertyConfig)
    starMirrorInterval

    targetShapeMirroredShapeSetMap: Map<number, Array<BaseShapeJS>> = new Map<number, Array<BaseShapeJS>>()
    mirroredShapeShapeEntryMap: Map<BaseShapeJS, ClonedShapeEntry> = new Map

    paperShapeGroup: paper.Group

    constructor(rawObj?, isMirage = false) {
        super(rawObj, isMirage);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape
    }

    getMirroredShapeArray(targetShape: BaseShapeJS):Array<BaseShapeJS>{
        let rawPtr = GetObjPtr(targetShape)

        if(!this.targetShapeMirroredShapeSetMap.has(rawPtr)){
            this.targetShapeMirroredShapeSetMap.set(rawPtr, new Array<BaseShapeJS>())
            let _this = this
            targetShape.registerValueChangeHandler("*")(()=>{
                for(let shape of _this.getMirroredShapeArray(targetShape)){
                    let entry = _this.mirroredShapeShapeEntryMap.get(shape)
                    updateEntry(entry)
                }
            })
        }

        return this.targetShapeMirroredShapeSetMap.get(rawPtr)
    }

    updateMirroredShapeArray(targetShape){
        let currentAngle = this.starMirrorInterval
        let mirroredShapeCount = Math.ceil(360 / this.starMirrorInterval) - 1
        let mirroredShapeArray = this.getMirroredShapeArray(targetShape)

        let currentShapeCount = mirroredShapeArray.length
        for(let currentShapeId = currentShapeCount ; currentShapeId < mirroredShapeCount; currentShapeId++){
            let duplicatedShape = createDuplication(targetShape, this.baseShape)
            mirroredShapeArray.push(duplicatedShape)
            this.mirroredShapeShapeEntryMap.set(duplicatedShape, {
                shape: duplicatedShape,
                targetShape: targetShape,
                centerObject: this.baseShape,
                angle: 0.0,
                isUsing: false
            })
        }

        for(let currentShapeId = mirroredShapeCount; currentShapeId < currentShapeCount; currentShapeId++){
            let duplicatedShape = mirroredShapeArray[currentShapeId]
            this.mirroredShapeShapeEntryMap.get(duplicatedShape).isUsing = false
        }

        let currentShapeId = 0
        while(currentAngle < 360){
            let duplicatedShape = mirroredShapeArray[currentShapeId]
            currentShapeId++
            this.mirroredShapeShapeEntryMap.set(duplicatedShape, {
                shape: duplicatedShape,
                targetShape:targetShape,
                centerObject: this.baseShape,
                angle: currentAngle,
                isUsing: true
            })

            let parentGroup = new paper.Group()
            parentGroup.applyMatrix = false
            parentGroup.data.meta = this.baseShape
            parentGroup.addChild(duplicatedShape.paperShape)

            this.paperShapeGroup.addChild(parentGroup)
            let _this = this
            duplicatedShape.getParent = function(){
                return _this.baseShape.getParent()
            }
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
                        let needCleanUp = false
                        for(let mirroredShape of this.getMirroredShapeArray(targetShape)){
                            // The shape has been moved to another store.
                            // Recreate all the mirages.
                            if(mirroredShape.getBornStoreId() != this.baseShape.getBornStoreId()){
                                needCleanUp = true
                            }
                        }

                        if(needCleanUp){
                            for(let mirroredShape of this.getMirroredShapeArray(targetShape)){
                                this.mirroredShapeShapeEntryMap.delete(mirroredShape)
                                mirroredShape.removePaperObj()
                            }
                            this.targetShapeMirroredShapeSetMap.set(GetObjPtr(targetShape), new Array())
                        }

                        this.updateMirroredShapeArray(targetShape)

                        for(let mirroredShape of this.getMirroredShapeArray(targetShape)){
                            let cloneShapeEntry = this.mirroredShapeShapeEntryMap.get(mirroredShape)
                            updateEntry(cloneShapeEntry)
                        }
                    }
                }
            }
        }
    }

    override cleanUp() {
        super.cleanUp();
        for(let [mirroredShape, entry] of this.mirroredShapeShapeEntryMap){
            mirroredShape.removePaperObj()
        }

        this.paperShapeGroup.remove()

        this.mirroredShapeShapeEntryMap = new Map
        this.targetShapeMirroredShapeSetMap = new Map
    }

    setInvisible() {
        super.setInvisible();

        for(let [mirroredShape, entry] of this.mirroredShapeShapeEntryMap){
            mirroredShape.paperShape.visible = false
            mirroredShape.selected = false
        }

    }
}

export {StarMirrorComponent}