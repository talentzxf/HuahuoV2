import {huahuoEngine} from "../EngineAPI";
import {Logger} from "hhcommoncomponents"
import {Vector2, relaxRectangle, pointsNear} from "hhcommoncomponents"
import * as paper from "paper";

declare var Module: any;

const BOUNDMARGIN: number = 10

class BaseShapeJS {
    protected rawObj: any = null;
    protected paperShape: paper.Path
    protected isSelected = false

    protected boundingBoxRect = null;

    public isPermanent: boolean = false;

    get selected(): boolean {
        return this.isSelected
    }

    set selected(val: boolean) {
        this.isSelected = val
    }

    get color(): paper.Color {
        let rawObjColor = this.rawObj.GetColor()
        return new paper.Color(rawObjColor.r, rawObjColor.g, rawObjColor.b, rawObjColor.a)
    }

    set color(val: paper.Color) {
        this.rawObj.SetColor(val.red, val.green, val.blue, val.alpha)
    }

    getLayer() {
        return this.rawObj.GetLayer()
    }

    awakeFromLoad() {
        this.isPermanent = true
        this.update();
    }

    getShapeName() {
        return "UnknownShape";
    }

    getPaperJs() {
        if (paper.project)
            return paper
        return window.paper
    }

    storeSegments(segments, keyframeId = null) {
        let segmentBuffer = []

        for (let id = 0; id < segments.length; id++) {
            segmentBuffer[6 * id] = segments[id].point.x
            segmentBuffer[6 * id + 1] = segments[id].point.y
            segmentBuffer[6 * id + 2] = segments[id].handleIn.x
            segmentBuffer[6 * id + 3] = segments[id].handleIn.y
            segmentBuffer[6 * id + 4] = segments[id].handleOut.x
            segmentBuffer[6 * id + 5] = segments[id].handleOut.y
        }

        if (keyframeId == null) // Set the current frame.
            this.rawObj.SetSegments(segmentBuffer, segments.length)
        else
            this.rawObj.SetSegmentsAtFrame(segmentBuffer, segments.length, keyframeId)
    }

    store() {
        let segments = this.paperShape.segments
        if (segments) {
            this.storeSegments(segments)
        }

        let zeroPointPosition = this.paperShape.localToGlobal(new paper.Point(0, 0))
        this.rawObj.SetPosition(zeroPointPosition.x, zeroPointPosition.y, 0)

        let scaling = this.getPaperShape().scaling
        this.rawObj.SetScale(scaling.x, scaling.y, 0)

        let rotation = this.getPaperShape().rotation
        this.rawObj.SetRotation(rotation)
    }

    constructor(rawObj?) {
        if (!rawObj) {
            let _this = this

            Logger.info("BaseShapeJS: Submitted execute method")
            huahuoEngine.ExecuteAfterInited(() => {

                Logger.info("BaseShapeJS: Executing raw obj creation method")
                _this.rawObj = Module.BaseShape.prototype.CreateShape(_this.getShapeName());
                Logger.info("BaseShapeJS: Executing afterWASMReady")
                _this.afterWASMReady();
                Logger.info("BaseShapeJS: Executed afterWASMReady")
            })
        } else {
            this.rawObj = rawObj
            this.afterWASMReady()
        }
    }

    isSelectable() {
        return true
    }

    afterWASMReady() {

    }

    getRawShape() {
        return this.rawObj
    }

    isVisible() {
        return this.rawObj.IsVisible();
    }

    beforeUpdate(updateOptions) {
        if (this.isPermanent && !this.rawObj.IsVisible()) {
            this.selected = false
        }
    }

    createShape() {
        throw "Can't create abstract shape, override this function."
    }

    duringUpdate(updateOptions) {
        if (updateOptions && updateOptions.updateShape) {
            if (!this.paperShape) {
                this.createShape()
            }
        }
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
            if (!newObj.divideAt(offset)) {
                Logger.error("ERRRRRRRR!!!")
            }

            this.storeSegments(newObj.segments, segmentKeyFrame.GetFrameId())
            newObj.removeSegments()
        }

        newObj.remove()
    }

    applySegments() {
        let segmentCount = this.rawObj.GetSegmentCount();
        if (segmentCount > 0 && this.paperShape.segments.length > 0) {
            let currentSegmentCount = this.paperShape.segments.length
            let createSegments = false
            if (currentSegmentCount != segmentCount) {
                this.paperShape.removeSegments()
                createSegments = true
            }

            for (let i = 0; i < segmentCount; i++) {
                let position = this.rawObj.GetSegmentPosition(i);
                let handleIn = this.rawObj.GetSegmentHandleIn(i);
                let handleOut = this.rawObj.GetSegmentHandleOut(i);

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

    afterUpdate(updateOptions) {
        if (updateOptions && updateOptions.updateShape) {
            this.applySegments()
            let scale = this.rawObj.GetScale()
            this.paperShape.scaling = new paper.Point(scale.x, scale.y)
            this.paperShape.rotation = this.rawObj.GetRotation()

            let pos = this.rawObj.GetPosition();// This position is the new global coordinate of the local (0,0).

            let currentZeroPoint = this.paperShape.localToGlobal(new paper.Point(0, 0))
            let currentCenter = this.paperShape.position

            let centerOffset = currentCenter.subtract(currentZeroPoint)

            this.paperShape.position = new paper.Point(pos.x, pos.y).add(new paper.Point(centerOffset))
        }

        if (this.isSelected) {
            if (updateOptions && updateOptions.updateBoundingBox) {
                if (this.boundingBoxRect)
                    this.boundingBoxRect.remove()

                let boundingBox = this.paperShape.bounds;

                let paperjs = this.getPaperJs()
                this.boundingBoxRect = new paperjs.Path.Rectangle(relaxRectangle(boundingBox, BOUNDMARGIN))
                this.boundingBoxRect.dashArray = [4, 10]
                this.boundingBoxRect.strokeColor = new paper.Color("black")
            }

            this.paperShape.selected = true
        } else {
            this.paperShape.selected = false
            if (updateOptions && updateOptions.updateBoundingBox) {
                if (this.boundingBoxRect)
                    this.boundingBoxRect.remove()
            }
        }
    }

    update(updateOptions = {updateShape: true, updateBoundingBox: true}) {
        this.beforeUpdate(updateOptions)
        this.duringUpdate(updateOptions)

        if (this.isPermanent == true && !this.rawObj.IsVisible()) {
            this.paperShape.visible = false
            this.isSelected = false
            this.paperShape.selected = false

            if (this.boundingBoxRect) {
                this.boundingBoxRect.remove();
            }
        } else {
            this.paperShape.visible = true
            this.afterUpdate(updateOptions)
        }
    }

    getPaperPoint(engineV3Point) {
        return new paper.Point(engineV3Point.x, engineV3Point.y)
    }

    getPaperShape(): paper.Path {
        return this.paperShape
    }
}

class ShapeFactory {
    shapeNameClzMap: Map<string, Function> = new Map<string, Function>();

    RegisterClass(shapeName: string, shapeConstructor: Function) {
        this.shapeNameClzMap.set(shapeName, shapeConstructor)
    }

    GetShapeConstructor(shapeName: string) {
        return this.shapeNameClzMap.get(shapeName)
    }
}

let shapeFactory = new ShapeFactory()

huahuoEngine.ExecuteAfterInited(() => {
    let baseShapeOnLoadHandler = new Module.ScriptEventHandlerImpl()
    baseShapeOnLoadHandler.handleEvent = function (baseShapeEventHandler) {
        let arg = Module.wrapPointer(baseShapeEventHandler, Module.ShapeLoadedEventArgs)
        let baseShape = arg.GetBaseShape();

        // Convention: Cpp class name is the JS class name.
        // TODO: Create a map of the shapename->JS class name mapping.

        let shapeConstructor = shapeFactory.GetShapeConstructor(baseShape.GetName())
        let newBaseShape = shapeConstructor(arg.GetBaseShape())

        newBaseShape.awakeFromLoad()

        let layer = newBaseShape.getLayer()
        huahuoEngine.getLayerShapes(layer).push(newBaseShape)
    }

    huahuoEngine.GetInstance().RegisterEvent("OnShapeLoaded", baseShapeOnLoadHandler)
})

export {BaseShapeJS, shapeFactory}