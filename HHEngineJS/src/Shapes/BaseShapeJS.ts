import {huahuoEngine} from "../EngineAPI";
import {relaxRectangle, PropertySheet, PropertyType, Logger} from "hhcommoncomponents"
import * as paper from "paper";
import {ShapeCenterSelector} from "./ShapeCenterSelector";

declare function castObject(obj: any, clz: any): any;

declare var Module: any;

const BOUNDMARGIN: number = 10

abstract class BaseShapeJS {
    protected rawObj: any = null;
    protected paperItem: paper.Item
    protected isSelected = false

    protected boundingBoxGroup = null;

    public isPermanent: boolean = false;

    private handlerId: number = 0 // always increment

    // This is used for Editor only to set properties.
    protected propertySheet: PropertySheet

    private valueChangeHandlersMap: Map<string, Array<Function>> = new Map()

    private parent: BaseShapeJS = null

    private bornStoreId: number = -1;

    private shapeCenterSelector: ShapeCenterSelector;

    get typename(): string{
        return this.rawObj.GetTypeName()
    }

    get pivotPosition(): paper.Point {
        return this.rawObj.GetGlobalPivotPosition()
    }

    set pivotPosition(centerPosition: paper.Point) {
        let localCenterPos = this.paperItem.globalToLocal(centerPosition)

        this.rawObj.SetGlobalPivotPosition(centerPosition.x, centerPosition.y, 0.0)
        this.rawObj.SetLocalPivotPosition(localCenterPos.x, localCenterPos.y, 0.0)
    }

    public getPointAt(offset): paper.Point{
        return this.paperShape.getPointAt(offset)
    }

    public getBornStoreId(): number {
        return this.bornStoreId
    }

    protected isUpdateStrokeColor() {
        return true;
    }

    set paperShape(val: paper.Path) {
        this.paperItem = val
    }

    get paperShape(): paper.Path {
        return this.paperItem as paper.Path
    }

    globalToLocal(globalPoint: paper.Point): paper.Point {
        return this.paperShape.globalToLocal(globalPoint)
    }

    localToGlobal(localPoint: paper.Point): paper.Point{
        return this.paperShape.localToGlobal(localPoint)
    }

    getNearestPoint(localPos: paper.Point) {
        if (this.paperShape.getNearestPoint){
            return this.paperShape.getNearestPoint(localPos)
        }
    }

    getGlobalNearestPoint(globalPos: paper.Point){
        if(this.paperShape.getNearestPoint){
            let localPos = this.paperShape.globalToLocal(globalPos)
            let localNearestPos = this.paperShape.getNearestPoint(localPos)
            return this.paperShape.localToGlobal(localNearestPos)
        }
    }

    getOffsetOf(localPos: paper.Point) {
        return this.paperShape.getOffsetOf(localPos)
    }

    divideAt(length: number) {
        return this.paperShape.divideAt(length)
    }

    get name(): string {
        return this.rawObj.GetName()
    }

    set name(name: string) {
        this.rawObj.SetName(name)
    }

    get bounds(): paper.Rectangle {
        return this.paperItem.bounds
    }

    get position(): paper.Point {
        return new paper.Point(this.pivotPosition.x, this.pivotPosition.y)
    }

    get rotation(): number {
        return this.rawObj.GetRotation()
    }

    get scaling(): paper.Point {
        return this.paperItem.scaling
    }

    setParent(parentShape: BaseShapeJS) {
        this.parent = parentShape
        parentShape.paperItem.addChild(this.paperItem)
    }

    getParent() {
        return this.parent
    }

    callHandlers(propertyName: string, val: any) {
        if (this.valueChangeHandlersMap.has(propertyName)) {
            for (let handler of this.valueChangeHandlersMap.get(propertyName).values()) {
                handler(val)
            }
        }
    }

    storeSameLayerShapes() {
        let parent = this.paperItem.parent
        if (parent) {
            for (let childShape of parent.children) {
                if (childShape.data && childShape.data.meta) {
                    let baseShape = childShape.data.meta
                    baseShape.store()
                }
            }
        }
    }

    duplicate() {
        let newRawObj = huahuoEngine.DuplicateObject(this.rawObj)
        let shapeLayer = newRawObj.GetLayer()

        let newShapeObj = huahuoEngine.getActivePlayer().getLayerShapes(shapeLayer).get(newRawObj.ptr)
        return newShapeObj
    }

    sendToBack() {
        this.paperItem.sendToBack()
        this.storeSameLayerShapes()
    }

    bringToFront() {
        this.paperItem.bringToFront()
        this.storeSameLayerShapes()
    }

    rotateAroundPivot(angle: number) {
        let zeroP = new paper.Point(0, 0)
        this.paperItem.rotate(angle, this.pivotPosition)

        let newRotationDegree = this.rawObj.GetRotation() + angle
        this.rawObj.SetRotation(newRotationDegree)

        this.callHandlers("rotation", newRotationDegree)
    }

    set position(val: paper.Point) {
        let curGlobalPivot = this.rawObj.GetGlobalPivotPosition()
        let curShapePosition = this.paperShape.position

        let offset = val.subtract(new paper.Point(curGlobalPivot.x, curGlobalPivot.y))
        let nextShapePosition = curShapePosition.add(offset)
        this.paperShape.position = nextShapePosition

        this.rawObj.SetGlobalPivotPosition(val.x, val.y, 0.0)
        let localPivotPosition = this.globalToLocal(val)
        this.rawObj.SetLocalPivotPosition(localPivotPosition.x, localPivotPosition.y, 0.0)
        this.callHandlers("position", val)
    }

    set scaling(val: paper.Point) {
        this.paperItem.scaling = val

        this.callHandlers("scaling", val)
    }

    set rotation(val: number) {
        this.rawObj.SetRotation(val)
        this.callHandlers("rotation", val)
    }

    get selected(): boolean {
        return this.isSelected
    }

    set selected(val: boolean) {
        this.isSelected = val

        this.updateBoundingBox()
    }

    updateBoundingBox() {
        if (this.isSelected) {
            {
                if (this.boundingBoxGroup)
                    this.boundingBoxGroup.remove()

                let boundingBox = this.paperItem.bounds;

                let paperjs = this.getPaperJs()
                this.boundingBoxGroup = new paperjs.Group()

                let boundingBoxRect = new paperjs.Path.Rectangle(relaxRectangle(boundingBox, BOUNDMARGIN))
                boundingBoxRect.dashArray = [4, 10]
                boundingBoxRect.strokeColor = new paper.Color("black")
                this.boundingBoxGroup.addChild(boundingBoxRect)

                let centerCircle = new paperjs.Path.Circle(this.pivotPosition, 10)
                centerCircle.fillColor = new paper.Color("red")
                centerCircle.data.meta = this.shapeCenterSelector

                this.boundingBoxGroup.addChild(centerCircle)
            }

            if (this.paperItem)
                this.paperItem.selected = true
        } else {
            if (this.paperItem)
                this.paperItem.selected = false
            if (this.boundingBoxGroup)
                this.boundingBoxGroup.remove()
            if (this.shapeCenterSelector)
                this.shapeCenterSelector.selected = false
        }
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

    // TODO: Do we really need a store ????
    store() {
        let segments = this.paperShape.segments
        if (segments) {
            this.storeSegments(segments)
        }

        let scaling = this.paperItem.scaling
        this.rawObj.SetScale(scaling.x, scaling.y, 0)

        // Store index
        let index = this.paperItem.index
        this.rawObj.SetIndex(index)

        this.updateBoundingBox()
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

    // Not sure how to pass getter/setter as functor.
    private getPosition() {
        return this.position
    }

    private setPosition(x: number, y: number) {
        this.paperItem.position = new paper.Point(x, y)
        this.store()
    }

    private getScaling() {
        return this.scaling
    }

    private setScaling(x: number, y: number) {
        this.paperItem.scaling = new paper.Point(x, y)
        this.store()
    }

    private getRotation(): number {
        return this.rotation
    }

    private setRotation(val: number) {
        this.rawObj.SetRotation(val)
        this.store()
    }

    // TODO: Move this into Cpp part
    followCurve: BaseShapeJS

    length(){
        return this.paperShape.length
    }

    getFollowCurve(){
        return this.followCurve
    }

    setFollowCurve(curve:BaseShapeJS){
        if(curve != this && curve != this.followCurve){
            this.followCurve = curve

            let curveStartPoint = this.followCurve.getPointAt(0)

            // let currentPivotPos = this.pivotPosition
            //
            // let offset = curveStartPoint.subtract(currentPivotPos)
            //
            // let newPosition = this.position.add(offset)

            this.position = this.followCurve.localToGlobal(curveStartPoint)
        }else{
            Logger.error("Can't bind the path !")
        }
    }

    atFollowCurveStart(){
        let curveStartPoint = this.followCurve.getPointAt(0)
        this.position = this.followCurve.localToGlobal(curveStartPoint)
    }

    atFollowCurveEnd(){
        let curveLength = this.followCurve.length()
        let curveEndPoint = this.followCurve.getPointAt(curveLength)
        this.position = this.followCurve.localToGlobal(curveEndPoint)
    }

    afterWASMReady() {
        this.rawObj = castObject(this.rawObj, Module[this.getShapeName()]);

        this.shapeCenterSelector = new ShapeCenterSelector(this)

        this.propertySheet = new PropertySheet();

        this.propertySheet.addProperty({
            key: "inspector.Type",
            type: PropertyType.STRING,
            getter: this.getTypeName.bind(this)
        })

        // Position
        this.propertySheet.addProperty({
            key: "inspector.Position",
            type: PropertyType.GROUP,
            children: [
                {
                    key: "inspector.FixedPosition",
                    type: PropertyType.VECTOR2,
                    getter: this.getPosition.bind(this),
                    setter: this.setPosition.bind(this),
                    registerValueChangeFunc: this.registerValueChangeHandler("position").bind(this),
                    unregisterValueChangeFunc: this.unregisterValueChangeHandler("position").bind(this)
                },
                {
                    key: "inspector.FollowPath",
                    type: PropertyType.PANEL,
                    children:[
                        {
                            key: "inspector.FollowTarget",
                            type: PropertyType.REFERENCE,
                            getter: this.getFollowCurve.bind(this),
                            setter: this.setFollowCurve.bind(this)
                        },
                        {
                            key:"inspector.AtBegin",
                            type: PropertyType.BUTTON,
                            action: this.atFollowCurveStart.bind(this)
                        },
                        {
                            key:"inspector.AtEnd",
                            type: PropertyType.BUTTON,
                            action: this.atFollowCurveEnd.bind(this)
                        }
                    ]
                },

            ]
        });

        this.propertySheet.addProperty({
            key: "inspector.Scaling",
            type: PropertyType.VECTOR2,
            getter: this.getScaling.bind(this),
            setter: this.setScaling.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("scaling").bind(this),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("scaling").bind(this)
        })

        this.propertySheet.addProperty({
            key: "inspector.Rotation",
            type: PropertyType.FLOAT,
            getter: this.getRotation.bind(this),
            setter: this.setRotation.bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("rotation").bind(this),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("rotation").bind(this)
        })
    }

    registerValueChangeHandler(valueName: string) {
        return function (valueChangedHandler: Function) {
            if (!this.valueChangeHandlersMap.has(valueName)) {
                this.valueChangeHandlersMap.set(valueName, new Map<Number, Function>())
            }

            let id = this.handlerId
            this.valueChangeHandlersMap.get(valueName).set(id, valueChangedHandler)
            this.handlerId++
            return id;
        }.bind(this)
    }

    unregisterValueChangeHandler(valueName: string) {
        return function (handlerId: number) {
            if (this.valueChangeHandlersMap.has(valueName)) {
                let valueChangeHandlerMap = this.valueChangeHandlersMap.get(valueName)
                valueChangeHandlerMap.delete(handlerId)
            }
        }
    }

    getPropertySheet() {
        return this.propertySheet
    }

    getRawShape() {
        return this.rawObj
    }

    isVisible() {
        return this.rawObj.IsVisible();
    }

    beforeUpdate() {
        if (this.isPermanent && !this.rawObj.IsVisible()) {
            this.selected = false
        }
    }

    createShape() {
        this.bornStoreId = huahuoEngine.GetCurrentStoreId()

        huahuoEngine.dispatchEvent("OnJSShapeCreated", this)
    }

    afterCreateShape() {
        if (!this.isPermanent) {
            let paperPos = this.paperShape.position

            let localPos = this.paperShape.globalToLocal(paperPos)

            this.rawObj.SetGlobalPivotPosition(paperPos.x, paperPos.y, 0.0);
            this.rawObj.SetLocalPivotPosition(localPos.x, localPos.y, 0.0);

        }
    }

    duringUpdate() {
        if (!this.paperItem) {
            this.createShape()
        }
    }

    getTypeName() {
        return this.rawObj.GetTypeName()
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
    }

    removeSegment(segment) {
        // Update all frames.
        this.rawObj.RemoveSegment(segment.index)

        segment.remove()

        this.updateBoundingBox()
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

    backCalculateZeroPoint(localPos: paper.Point, globalPos: paper.Point, radian: number) {
        let OB = localPos.x + localPos.y * Math.tan(radian)
        let OC = OB * Math.cos(radian)
        let zx = globalPos.x - OC

        let PB = localPos.y / Math.cos(radian)
        let CB = OB * Math.sin(radian)
        let PC = PB - CB

        let zy = globalPos.y - PC

        return new paper.Point(zx, zy)
    }

    afterUpdate() {
        this.applySegments()
        let scale = this.rawObj.GetScale()
        this.scaling = new paper.Point(scale.x, scale.y)

        this.paperItem.position = new paper.Point(0.0, 0.0)
        // Reset the rotation.
        this.paperItem.rotation = this.rawObj.GetRotation();
        // Rotation around zero point.

        // The coordinate should have been aligned now.
        let globalPivotPosition = this.rawObj.GetGlobalPivotPosition()
        let localPivotPosition = this.rawObj.GetLocalPivotPosition()

        let radian = this.rawObj.GetRotation() / 180 * Math.PI

        let shapeZero = this.backCalculateZeroPoint(localPivotPosition, globalPivotPosition, -radian)

        let offset = this.paperShape.localToGlobal(new paper.Point(0, 0)) // As position is already (0,0). The global position of (0,0) indicates the center of the bounding rect.

        let newPosition = shapeZero.subtract(offset)
        if (this.paperItem.parent)
            this.paperItem.position = this.paperItem.parent.globalToLocal(newPosition)
        else
            this.paperItem.position = newPosition

        // this.paperItem.position = new paper.Point(0, 0)
        // this.paperItem.rotation = 0
        //
        // let pos = this.rawObj.GetPosition();// This position is the new global coordinate of the local (0,0).
        // let currentZeroPoint = this.paperItem.localToParent(new paper.Point(0, 0))
        // let currentCenter = this.position
        //
        // let centerOffset = currentCenter.subtract(currentZeroPoint)
        //
        // let candidatePosition = new paper.Point(pos.x, pos.y).add(new paper.Point(centerOffset))
        // this.position = candidatePosition

        // Adjust index
        if (this.paperItem.index != this.rawObj.GetIndex() && this.paperItem.index > 0) {
            let parent = this.paperItem.parent
            if (parent) {
                parent.insertChild(this.rawObj.GetIndex(), this.paperItem)
            }
        }
    }

    update() {
        this.beforeUpdate()
        this.duringUpdate()

        if (this.isPermanent == true && !this.rawObj.IsVisible()) {
            this.paperItem.visible = false
            this.selected = false
        } else {
            this.paperItem.visible = true
            this.afterUpdate()
        }

        this.updateBoundingBox()
    }

    getPaperPoint(engineV3Point) {
        return new paper.Point(engineV3Point.x, engineV3Point.y)
    }

    remove() {
        // TODO: TODO
        huahuoEngine.DestroyShape(this.rawObj)
        this.paperItem.remove()

        this.boundingBoxGroup.remove()

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

let shapeFactory = window["shapeFactory"]
if (!shapeFactory) {
    shapeFactory = new ShapeFactory()
    window["shapeFactory"] = shapeFactory
}

huahuoEngine.ExecuteAfterInited(() => {
    let eventName = "OnShapeLoaded"
    if (huahuoEngine.GetInstance().IsEventRegistered(eventName))
        return;

    let baseShapeOnLoadHandler = new Module.ScriptEventHandlerImpl()
    baseShapeOnLoadHandler.handleEvent = function (baseShapeEventHandler) {
        let arg = Module.wrapPointer(baseShapeEventHandler, Module.ShapeLoadedEventArgs)
        let baseShape = arg.GetBaseShape();

        let shapeStoreId = baseShape.GetLayer().GetObjectStore().GetStoreId()
        if (shapeStoreId != huahuoEngine.GetCurrentStoreId()) {
            let elementShapes = huahuoEngine.GetElementShapeByStoreId(shapeStoreId)
            if (elementShapes) {
                for (let elementShape of elementShapes) {
                    if (elementShape) {
                        elementShape.update();
                    }
                }
            }
            return;
        }

        // Convention: Cpp class name is the JS class name.
        // TODO: Create a map of the shapename->JS class name mapping.

        let shapeConstructor = shapeFactory.GetShapeConstructor(baseShape.GetTypeName())
        let newBaseShape = shapeConstructor(arg.GetBaseShape())

        newBaseShape.awakeFromLoad()

        let layer = newBaseShape.getLayer()
        huahuoEngine.getActivePlayer().getLayerShapes(layer).set(newBaseShape.getRawShape().ptr, newBaseShape)
    }

    huahuoEngine.GetInstance().RegisterEvent(eventName, baseShapeOnLoadHandler)
})

export {BaseShapeJS, shapeFactory}