import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {mirrorPoint, Logger} from "hhcommoncomponents";
import * as paper from "paper"
import {clzObjectFactory} from "../CppClassObjectFactory";

let componentName = "MirrorComponent"
class MirrorComponent extends AbstractComponent {

    static createComponent(rawObj) {
        return new MirrorComponent(rawObj)
    }

    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    targetShapeMirroredShapeMap: Map<number, BaseShapeJS> = new Map<number, BaseShapeJS>()

    paperShapeGroup: paper.Group
    p1: paper.Point
    p2: paper.Point

    constructor(rawObj?) {
        super(rawObj)

        this.rawObj.SetTypeName(componentName)

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape

        // let line1 = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(1000, 0)) // X Axis
        // let line2 = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(0, 1000)) // Y Axis
        //
        // line1.strokeColor = new paper.Color("red")
        // line1.strokeWidth = 1
        // line2.strokeColor = new paper.Color("green")
        // line2.strokeWidth = 1
        //
        // this.paperShapeGroup.addChild(line1)
        // this.paperShapeGroup.addChild(line2)
    }


    setBaseShape(baseShape: BaseShapeJS) {
        super.setBaseShape(baseShape);

        let baseShapeSegments = this.baseShape.getSegments()
        if(baseShapeSegments != null && baseShapeSegments.length == 2) {
            this.p1 = this.baseShape.getSegments()[0].point
            this.p2 = this.baseShape.getSegments()[1].point
        }
    }

    private _createDuplication(shape){
        /*
        let duplicatedShape = targetShape.duplicate()
        // Move the position to the target position.
        duplicatedShape.position = mirrorPoint(duplicatedShape.position,
            this.p1, this.p2)

        duplicatedShape.setSelectedMeta(this.baseShape)
        duplicatedShape.setIsMovable(false)
        return duplicatedShape
         */

        let rawObj = shape.rawObj
        let shapeConstructor = clzObjectFactory.GetClassConstructor(rawObj.GetTypeName())
        let duplicatedShape = shapeConstructor(rawObj)
        duplicatedShape.setSelectedMeta(this.baseShape)
        // duplicatedShape.setIsMovable(false)

        duplicatedShape.registerValueChangeHandler("*")(()=>{
            shape.update() // update the original shape.
        })

        duplicatedShape.awakeFromLoad() // Awake it, ensure it didn't overridden positions.
        this.paperShapeGroup.addChild(duplicatedShape.paperItem)

        return duplicatedShape
    }

    createDuplication(shape){
        let duplicatedShape = this._createDuplication(shape)
        this.targetShapeMirroredShapeMap.set(shape.rawObj.ptr, duplicatedShape)

        this.paperShapeGroup.addChild(duplicatedShape.paperItem)

        let _this = this
        duplicatedShape.getParent = function(){
            return _this.baseShape.getParent()
        }

        shape.registerValueChangeHandler("*")(()=>{
            duplicatedShape.update()
        })
        return duplicatedShape
    }

    cleanUp(){
        for(let [targetShapePtr, mirroredShape] of this.targetShapeMirroredShapeMap){
            mirroredShape.removePaperObj()
        }

        this.targetShapeMirroredShapeMap = new Map<number, BaseShapeJS>()
    }

    afterUpdate() {
        super.afterUpdate();

        let baseShapeParent = this.baseShape.paperShape.parent
        if(baseShapeParent != null && this.paperShapeGroup.parent != baseShapeParent)
            baseShapeParent.addChild(this.paperShapeGroup)

        let segments = this.baseShape.getSegments()

        if(segments == null || segments.length != 2){ // The base shape is not ready!
            return;
        }

        // Update p1 and p2 according to the updated position.
        this.p1 = this.baseShape.localToParent( segments[0].point )
        this.p2 = this.baseShape.localToParent( segments[1].point )

        // Rotate the coordinate
        let mirroredX = mirrorPoint(new paper.Point(1,0), this.p1, this.p2)
        let mirroredZero = mirrorPoint(new paper.Point(0,0), this.p1, this.p2)

        let radian = -Math.atan2(mirroredX.y - mirroredZero.y, mirroredX.x - mirroredZero.x)

        // this.paperShapeGroup.scaling.x = -1
        this.paperShapeGroup.scaling.y = -1

        // Convert to angle
        this.paperShapeGroup.rotation = -radian/ Math.PI * 180

        // Get the current offset of the group shape.
        let offset = this.paperShapeGroup.position.subtract(this.paperShapeGroup.localToParent(new paper.Point(0,0)))

        let newPosition = mirroredZero.add(offset)
        this.paperShapeGroup.position = newPosition

        // this.paperShapeGroup.scaling = new paper.Point(0.0, -1.0)

        if (this.targetShapeArray) {
            // Check if all target shapes are mirrored
            for (let targetShape of this.targetShapeArray) {
                if(targetShape != null){ // Target shape might be null if the target shape has not been loaded yet.
                    if (!this.targetShapeMirroredShapeMap.has(targetShape.rawObj.ptr)) {
                        this.createDuplication(targetShape)
                    }

                    let duplicatedShape = this.targetShapeMirroredShapeMap.get(targetShape.rawObj.ptr)
                    if(duplicatedShape.getBornStoreId() != this.baseShape.getBornStoreId()){
                        duplicatedShape.removePaperObj()
                        duplicatedShape = this.createDuplication(targetShape)
                    }

                    duplicatedShape.update()
                }
            }
        }
    }
}
clzObjectFactory.RegisterClass(componentName, MirrorComponent.createComponent)

export {MirrorComponent}