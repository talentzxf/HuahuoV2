import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {paper, BaseShapeJS} from "hhenginejs";

class ShapeMorphHander extends ShapeTranslateMorphBase{
    curSegment: paper.Segment
    curSegmentStartPos: paper.Point

    targetShape: BaseShapeJS
    constructor() {
        super();
    }

    setSegment(hitSegment:paper.Segment){
        this.curSegment = hitSegment
        this.curSegmentStartPos = this.curSegment.point.clone()

        hitSegment.selected = true
        hitSegment.handleIn.selected = true
        hitSegment.handleOut.selected = true
    }

    beginMove(startPos, hitResult) {
        if(this.curObjs.size != 1){
            throw "Can't morph multiple objects!!!"
        }

        this.targetShape = this.curObjs.values().next().value // There's only one object in the set, get it.
        super.beginMove(startPos);
        this.setSegment(hitResult.segment)
    }

    dragging(pos){
        if(this.isDragging && this.curSegment != null){
            let offset = pos.subtract(this.startPos)

            let proposedNewPosition = this.curSegmentStartPos.add(offset)
            this.curSegment.point = proposedNewPosition;

            console.log("Dragging to new pos:" + proposedNewPosition)

            // After morph, the position of the shape might be shifted, so we need to store the new position in the Cpp side.
            this.targetShape.store({position: true, segments: true})
            this.targetShape.update( {updateShape: false, updateBoundingBox : true});
        }
    }
}

class ShapeHandlerMoveHandler extends ShapeMorphHander{
    targetHandleName: string = ""
    targetHandleStartPos: paper.Point = null
    beginMove(startPos, hitResult) {
        super.beginMove(startPos, hitResult);
        if(hitResult.type == "handle-in")
            this.targetHandleName = "handleIn"
        else
            this.targetHandleName = "handleOut"

        this.targetHandleStartPos = this.curSegment[this.targetHandleName].clone()
    }

    dragging(pos){
        if(this.isDragging && this.curSegment != null){
            let offset = pos.subtract(this.startPos)
            let targetHandlePos = this.targetHandleStartPos.add(offset)

            this.curSegment[this.targetHandleName] = targetHandlePos

            // After morph, the position of the shape might be shifted, so we need to store the new position in the Cpp side.
            this.targetShape.store({position: false, segments: true})
            this.targetShape.update( {updateShape: false, updateBoundingBox : true});
        }
    }
}

let shapeHandlerMoveHandler = new ShapeHandlerMoveHandler()
let shapeMorphHandler = new ShapeMorphHander()
export {shapeMorphHandler, shapeHandlerMoveHandler}