import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {paper, BaseShapeJS} from "hhenginejs";

class ShapeMorphHandler extends ShapeTranslateMorphBase{
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

    beginMove(startPos, hitResult = null) {
        if(this.curObjs.size != 1){
            throw "Can't morph multiple objects!!!"
        }

        this.targetShape = this.curObjs.values().next().value // There's only one object in the set, get it.
        super.beginMove(startPos);

        if(hitResult != null && hitResult.segment != null)
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

class ShapeHandlerMoveHandler extends ShapeMorphHandler{
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

            this.targetShape.store()
            this.targetShape.update( {updateShape: false, updateBoundingBox : true});
        }
    }
}

class ShapeInsertSegmentHandler extends ShapeMorphHandler{
    circle: paper.Path
    beginMove(startPos) {
        super.beginMove(startPos);

        let obj:paper.Path = this.targetShape.getPaperShape()
        let localPos = obj.globalToLocal(startPos)
        let nearestPoint = obj.getNearestPoint(localPos)
        let offset = obj.getOffsetOf(nearestPoint)

        let newSegment = obj.divideAt(offset)
        this.setSegment(newSegment)

        this.targetShape.insertSegment(localPos)
    }
}

let shapeHandlerMoveHandler = new ShapeHandlerMoveHandler()
let shapeMorphHandler = new ShapeMorphHandler()
let shapeInsertSegmentHandler = new ShapeInsertSegmentHandler()
export {shapeMorphHandler, shapeHandlerMoveHandler, shapeInsertSegmentHandler}