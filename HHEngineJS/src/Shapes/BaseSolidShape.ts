import {BaseShapeJS, eps} from "./BaseShapeJS";
import {StrokeComponent} from "../Components/StrokeComponent";
import {FillColorComponent} from "../Components/FillColorComponent";
import {PropertyType, Vector2} from "hhcommoncomponents";

abstract class BaseSolidShape extends BaseShapeJS {
    override initShapeFromEditor() {
        super.initShapeFromEditor();

        this.addComponent(new StrokeComponent())
        this.addComponent(new FillColorComponent())
    }


    setSegmentProperty(idx, property, value) {
        let segment = this.paperShape.segments[idx]
        let currentValue = this.paperShape.segments[idx][property]

        if (Math.abs(currentValue - value) < eps)
            return
        this.paperShape.segments[idx][property] = value
        this.store()

        this.callHandlers("segments", {idx: idx, property: property, value: value})
    }

    restoreFrameSegmentsBuffer(frameSegmentsBuffer) {
        for (let keyframeObj of frameSegmentsBuffer) {
            let frameId = keyframeObj["frameId"]
            let segments = keyframeObj["segments"]
            for (let segmentIdx = 0; segmentIdx < segments.length; segmentIdx++) {
                this.storeSegments(segments, frameId)
            }
        }

        this.update(true)

        this.callHandlers("segments", null)
    }

    getFrameIdSegmentsBuffer() {
        let frameSegments = []
        let keyFrameCount = this.rawObj.GetSegmentKeyFrameCount()

        for (let keyFrameIdx = 0; keyFrameIdx < keyFrameCount; keyFrameIdx++) {
            let segmentKeyFrame = this.rawObj.GetSegmentKeyFrameAtKeyFrameIndex(keyFrameIdx);
            let segmentCount = segmentKeyFrame.GetTotalSegments()

            let frameObj = {
                frameId: segmentKeyFrame.GetFrameId(),
                segments: []
            }
            for (let segmentIdx = 0; segmentIdx < segmentCount; segmentIdx++) {
                let newSegmentBuffer = {
                    point: Vector2.fromObj(segmentKeyFrame.GetPosition(segmentIdx)),
                    handleIn: Vector2.fromObj(segmentKeyFrame.GetHandleIn(segmentIdx)),
                    handleOut: Vector2.fromObj(segmentKeyFrame.GetHandleOut(segmentIdx))
                }

                frameObj.segments.push(newSegmentBuffer)
            }

            frameSegments.push(frameObj)
        }

        return frameSegments
    }

    getSegmentsBuffer(segments, keyFrameId = null) {
        let segmentBuffer = []
        if (keyFrameId == null) {
            for (let id = 0; id < segments.length; id++) {
                segmentBuffer[6 * id] = segments[id].point.x
                segmentBuffer[6 * id + 1] = segments[id].point.y
                segmentBuffer[6 * id + 2] = segments[id].handleIn.x
                segmentBuffer[6 * id + 3] = segments[id].handleIn.y
                segmentBuffer[6 * id + 4] = segments[id].handleOut.x
                segmentBuffer[6 * id + 5] = segments[id].handleOut.y
            }
        }

        return segmentBuffer
    }

    storeSegments(segments, keyframeId = null) {

        let segmentBuffer = this.getSegmentsBuffer(segments)

        if (keyframeId == null) // Set the current frame.
            this.rawObj.SetSegments(segmentBuffer, segments.length)
        else
            this.rawObj.SetSegmentsAtFrame(segmentBuffer, segments.length, keyframeId)
    }

    // This might be overridden by CurveShape. Because we want to implement the growth factor.
    getSegments() {
        if (!this.paperShape)
            return null;

        return this.paperShape.segments
    }

    insertSegment(localPos: paper.Point) { // Need to add segments in all keyframes
        let keyFrameCount = this.rawObj.GetSegmentKeyFrameCount()
        if (keyFrameCount <= 0)
            return;

        let newObj = new paper.Path() // Clone the object at keyFrameIdx and add new segment and save.
        newObj.applyMatrix = false
        newObj.visible = false
        newObj.closed = true

        // TODO: 1.Merge this with applySegments??
        // TODO: 2.Add a clone method.
        for (let keyFrameIdx = 0; keyFrameIdx < keyFrameCount; keyFrameIdx++) {
            let segmentKeyFrame = this.rawObj.GetSegmentKeyFrameAtKeyFrameIndex(keyFrameIdx);
            let segmentCount = segmentKeyFrame.GetTotalSegments()
            for (let segmentIdx = 0; segmentIdx < segmentCount; segmentIdx++) {
                let position = segmentKeyFrame.GetPosition(segmentIdx)
                let handleIn = segmentKeyFrame.GetHandleIn(segmentIdx)
                let handleOut = segmentKeyFrame.GetHandleOut(segmentIdx)

                let positionPoint = new paper.Point(position.x, position.y)
                let handleInPoint = new paper.Point(handleIn.x, handleIn.y)
                let handleOutPoint = new paper.Point(handleOut.x, handleOut.y)

                let newSegment = new paper.Segment(positionPoint, handleInPoint, handleOutPoint)
                newObj.add(newSegment)
            }

            let nearestPoint = newObj.getNearestPoint(localPos)
            let offset = newObj.getOffsetOf(nearestPoint)

            while (!newObj.divideAt(offset)) {
                offset += 0.01 // Hit the corner points, offset a little and divide again.
            }

            this.storeSegments(newObj.segments, segmentKeyFrame.GetFrameId())
            newObj.removeSegments()
        }

        newObj.remove()

        this.callHandlers("segment", null)
    }

    removeSegment(segment) {
        // Update all frames.
        this.rawObj.RemoveSegment(segment.index)

        segment.remove()

        this.callHandlers("segment", null)
    }

    getSegmentPoint(idx) {
        if (this.getActor() && this.getActor().getSegmentActor() && this.getActor().getSegmentActor().isPointValid(idx)) {
            return this.getActor().getSegmentActor().getPoint(idx)
        }

        return this.rawObj.GetSegmentPosition(idx);
    }

    getSegmentHandleIn(idx) {
        if (this.getActor() && this.getActor().getSegmentActor() && this.getActor().getSegmentActor().isHandleInValid(idx)) {
            return this.getActor().getSegmentActor().getHandleIn(idx)
        }

        return this.rawObj.GetSegmentHandleIn(idx);
    }

    getSegmentHandleOut(idx) {
        if (this.getActor() && this.getActor().getSegmentActor() && this.getActor().getSegmentActor().isHandleOutValid(idx)) {
            return this.getActor().getSegmentActor().getHandleOut(idx)
        }

        return this.rawObj.GetSegmentHandleOut(idx);
    }

    applySegments() {
        let segmentCount = this.rawObj.GetSegmentCount();
        if (segmentCount > 0 && this.paperShape.segments != null) {
            let currentSegmentCount = this.paperShape.segments.length
            let createSegments = false
            if (currentSegmentCount != segmentCount) {
                this.paperShape.removeSegments()
                createSegments = true
            }

            for (let i = 0; i < segmentCount; i++) {
                let position = this.getSegmentPoint(i)
                let handleIn = this.getSegmentHandleIn(i)
                let handleOut = this.getSegmentHandleOut(i)

                let positionPoint = new paper.Point(position.x, position.y)
                let handleInPoint = new paper.Point(handleIn.x, handleIn.y)
                let handleOutPoint = new paper.Point(handleOut.x, handleOut.y)

                if (createSegments) {
                    this.paperShape.insert(i, new paper.Segment(positionPoint, handleInPoint, handleOutPoint))
                } else {
                    this.paperShape.segments[i].point = positionPoint
                    this.paperShape.segments[i].handleIn = handleInPoint
                    this.paperShape.segments[i].handleOut = handleOutPoint
                }
            }
            return true
        }

        return false
    }

    store() {
        let segments = this.getSegments()
        if (segments) {
            this.storeSegments(segments)
        }
        super.store();
    }

    afterWASMReady() {
        super.afterWASMReady();

        let segmentComponentProperty = {
            key: "Segment",
            type: PropertyType.COMPONENT,
            config: {
                children: [{
                    type: PropertyType.ARRAY,
                    elementType: PropertyType.SEGMENT,
                    getter: () => {
                        return this.getSegments()
                    }
                }]
            }
        }

        this.propertySheet.addProperty(segmentComponentProperty)
    }

    afterUpdate(force: boolean = false) {
        this.applySegments() // Need to apply segments before other update operations.
        super.afterUpdate(force);
    }
}

export {BaseSolidShape}