import {AbstractComponent, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {mirrorPoint, Logger} from "hhcommoncomponents";
import * as paper from "paper"
import {clzObjectFactory} from "../CppClassObjectFactory";

class MirrorComponent extends AbstractComponent {
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    targetShapeMirroredShapeMap: Map<BaseShapeJS, BaseShapeJS> = new Map<BaseShapeJS, BaseShapeJS>()

    paperShapeGroup: paper.Group
    p1: paper.Point
    p2: paper.Point

    constructor(rawObj?) {
        super(rawObj)

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape

        let line1 = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(0, 1000))
        let line2 = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(1000, 0))

        line1.strokeColor = new paper.Color("green")
        line1.strokeWidth = 1
        line2.strokeColor = new paper.Color("red")
        line2.strokeWidth = 1

        this.paperShapeGroup.addChild(line1)
        this.paperShapeGroup.addChild(line2)
    }

    setStartPoint(p1) {
        this.p1 = p1
    }

    setEndPoint(p2) {
        this.p2 = p2
    }

    createDuplication(shape){
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

        duplicatedShape.update()

        this.paperShapeGroup.addChild(duplicatedShape.paperItem)

        return duplicatedShape
    }

    afterUpdate() {
        super.afterUpdate();

        let segments = this.baseShape.getSegments()

        if(segments.length != 2){
            Logger.error("Why a mirror don't have two segments??")
            return;
        }

        // Update p1 and p2 according to the updated position.
        this.p1 = this.baseShape.localToGlobal( segments[0].point )
        this.p2 = this.baseShape.localToGlobal( segments[1].point )

        // Get the current offset of the group shape.
        let offset = this.paperShapeGroup.position.subtract(this.paperShapeGroup.localToGlobal(new paper.Point(0,0)))
        let mirroredZero = mirrorPoint(new paper.Point(0,0), this.p1, this.p2)

        let newPosition = mirroredZero.add(offset)
        this.paperShapeGroup.position = newPosition

        if (this.targetShapeArray) {
            // Check if all target shapes are mirrored
            for (let targetShape of this.targetShapeArray) {
                if (!this.targetShapeMirroredShapeMap.has(targetShape)) {
                    let duplicatedShape = this.createDuplication(targetShape)

                    this.targetShapeMirroredShapeMap.set(targetShape, duplicatedShape)

                    this.paperShapeGroup.addChild(duplicatedShape.paperItem)
                }

                let duplicatedShape = this.targetShapeMirroredShapeMap.get(targetShape)
                duplicatedShape.update()
            }
        }
    }

}

export {MirrorComponent}