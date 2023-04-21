import {huahuoEngine} from "../EngineAPI";
import {Vector2, eventBus} from "hhcommoncomponents";
import {relaxRectangle, PropertySheet, PropertyType, Logger} from "hhcommoncomponents"
import * as paper from "paper";
import {ShapeCenterSelector} from "./ShapeCenterSelector";
import {ValueChangeHandler} from "./ValueChangeHandler";
import {AbstractComponent} from "../Components/AbstractComponent";
import {BaseShapeActions} from "../EventGraph/BaseShapeActions";

let BASIC_COMPONENTS = "BasicComponents"

let basicComponents = ["ShapeTransformFrameState", "ShapeSegmentFrameState"]

declare function castObject(obj: any, clz: any): any;

declare var Module: any;

declare class ShapeFollowCurveFrameState {
    GetTargetShape()

    GetLengthRatio(): number

    RecordTargetShape(frameId: number, targetCurve)

    RecordLengthRatio(frameId: number, lengthRatio: number)
}

const BOUNDMARGIN: number = 10
const eps: number = 0.001

let totallyUpdated: number = 0

abstract class BaseShapeJS {
    protected rawObj: any = null;
    protected paperItem: paper.Item
    protected isSelected = false

    protected boundingBoxGroup = null;

    // If this shape is not loaded from (i.e. generated on the fly), we might need to record some information during creation. like it's local pivot position.
    // Or else, just read these information from the file.
    private isLoadedFromFile: boolean = false;

    // This is used for Editor only to set properties.
    protected propertySheet: PropertySheet

    private parent: BaseShapeJS = null

    private bornStoreId: number = -1;

    private shapeCenterSelector: ShapeCenterSelector;

    private shapeFollowCurveFrameState: ShapeFollowCurveFrameState;

    private valueChangeHandler: ValueChangeHandler = new ValueChangeHandler()

    private customComponents: Array<AbstractComponent> = new Array<AbstractComponent>()

    private _isMirage: boolean = false
    private _isUpdatePos: boolean = true

    private lastRenderFrame = -1

    private followCurveEventRegistered = false

    // Purpose of action is to store temporary results during system running. All the status in the action won't be persisted.
    private action: BaseShapeActions = new BaseShapeActions(this)

    public getAction() {
        return this.action
    }

    set isTransformationPermanent(isPermanent: boolean) {
        this.rawObj.SetRecordTransformationOfKeyFrame(isPermanent)
    }

    set isUpdatePos(val: boolean) {
        this._isUpdatePos = val
    }

    set isMirage(val: boolean) {
        this._isMirage = val
    }

    get isMirage() {
        return this._isMirage
    }

    get belongStoreId(): number {
        return this.rawObj.GetStoreId()
    }

    get typename(): string {
        return this.rawObj.GetTypeName()
    }

    get pivotPosition(): paper.Point {
        if (!this.followCurve) {
            if (this.action.isPositionValid)
                return this.action.position
            return this.rawObj.GetGlobalPivotPosition()
        }

        let lengthRatio = this.shapeFollowCurveFrameState.GetLengthRatio();

        let totalLength = this.followCurve.length()
        let targetLength = totalLength * lengthRatio

        let curvePoint = this.followCurve.getPointAt(targetLength)
        return this.followCurve.localToGlobal(curvePoint)
    }

    set pivotPosition(centerPosition: paper.Point) {
        let currentScaling = this.paperItem.scaling

        this.paperItem.scaling = new paper.Point(1.0, 1.0)

        let localCenterPos = this.paperItem.globalToLocal(centerPosition)

        this.rawObj.SetGlobalPivotPosition(centerPosition.x, centerPosition.y, 0.0)
        this.rawObj.SetLocalPivotPosition(localCenterPos.x, localCenterPos.y, 0.0)

        this.paperItem.scaling = currentScaling
    }

    addComponent(component: AbstractComponent, persistentTheComponent: boolean = true) {
        if (persistentTheComponent)
            this.rawObj.AddFrameState(component.rawObj)
        component.setBaseShape(this)
        this.customComponents.push(component)

        component.initPropertySheet(this.propertySheet)
    }

    getComponentCountByTypeName(typeName) {
        let returnCount = 0
        for (let component of this.customComponents) {
            if (component.getTypeName() == typeName) {
                returnCount++
            }
        }
        return returnCount
    }

    getComponentByTypeName(typeName) {
        for (let component of this.customComponents) {
            if (component.getTypeName() == typeName) {
                return component
            }
        }
        return null
    }

    public getPointAt(offset): paper.Point {
        return this.paperShape.getPointAt(offset)
    }

    public getBornStoreId(): number {
        return this.bornStoreId
    }

    public setBornStoreId(val: number) {
        this.bornStoreId = val
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

    localToGlobal(localPoint: paper.Point): paper.Point {
        return this.paperShape.localToGlobal(localPoint)
    }

    localToParent(localPoint: paper.Point): paper.Point {
        return this.paperShape.localToParent(localPoint)
    }

    getNearestPoint(localPos: paper.Point) {
        if (this.paperShape.getNearestPoint) {
            return this.paperShape.getNearestPoint(localPos)
        }
    }

    getGlobalNearestPoint(globalPos: paper.Point) {
        if (this.paperShape.getNearestPoint) {
            let localPos = this.paperShape.globalToLocal(globalPos)
            let localNearestPos = this.paperShape.getNearestPoint(localPos)
            return this.paperShape.localToGlobal(localNearestPos)
        }
    }

    getOffsetOf(localPos: paper.Point) {
        return this.paperShape.getOffsetOf(localPos)
    }

    getGlobalOffsetOf(globalPos: paper.Point) {
        let localPos = this.paperShape.globalToLocal(globalPos)
        let localNearestPos = this.paperShape.getNearestPoint(localPos)
        return this.getOffsetOf(localNearestPos)
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

    // The position is still in the parent's local coordinate.
    // The position might be different from papershape's position !!
    // The position is fixed after the shape is created until explicitly change while the paper's position might change is the shape is morphed.
    get position(): paper.Point {
        return new paper.Point(this.pivotPosition.x, this.pivotPosition.y)
    }

    get shapePosition(): paper.Point {
        return this.paperShape.position
    }

    get rotation(): number {
        if (this.action.isRotationValid)
            return this.action.rotation
        return this.rawObj.GetRotation()
    }

    get scaling(): paper.Point {
        let scaling = this.rawObj.GetScale()
        return new paper.Point(scaling.x, scaling.y)
    }

    get bornFrameId() {
        return this.rawObj.GetBornFrameId()
    }

    set bornFrameId(val: number) {
        this.rawObj.SetBornFrameId(val)
    }

    addAnimationOffset(offset) {
        this.rawObj.AddAnimationOffset(offset)
    }

    setParent(parentShape: BaseShapeJS) {
        this.parent = parentShape
        parentShape.paperItem.addChild(this.paperItem)
    }

    getParent() {
        return this.parent
    }

    movable: boolean = true
    selectedMeta: BaseShapeJS = null

    isMovable() {
        return this.movable
    }

    setIsMovable(val: boolean) {
        this.movable = val
    }

    setSelectedMeta(baseShape) {
        this.selectedMeta = baseShape
    }

    storeSameLayerShapeIndices() {
        let parent = this.paperItem.parent
        if (parent) {
            for (let childShape of parent.children) {
                if (childShape.data && childShape.data.meta) {
                    let baseShape = childShape.data.meta

                    let shapeIndex = baseShape.paperItem.index
                    baseShape.rawObj.SetIndex(shapeIndex)
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
        this.storeSameLayerShapeIndices()
        this.valueChangeHandler.callHandlers("index", this.paperItem.index)
    }

    bringToFront() {
        this.paperItem.bringToFront()
        this.storeSameLayerShapeIndices()
        this.valueChangeHandler.callHandlers("index", this.paperItem.index)
    }

    rotateAroundPivot(angle: number, persist: boolean = true) {
        let zeroP = new paper.Point(0, 0)
        this.paperItem.rotate(angle, this.pivotPosition)

        if (persist) {
            let newRotationDegree = this.rawObj.GetRotation() + angle
            this.rawObj.SetRotation(newRotationDegree)

            this.valueChangeHandler.callHandlers("rotation", newRotationDegree)
        }
    }

    // The method can only be called after the shape has been created.
    setParentLocalPosition(val: paper.Point, callHandlers: boolean = true, forceUpdate: boolean = true, isUpdatefollowCurve: boolean = true) {
        let currentScaling = this.scaling

        try {
            this.paperItem.scaling = new paper.Point(1.0, 1.0)

            let curGlobalPivot = this.rawObj.GetGlobalPivotPosition()
            let curShapePosition = this.paperShape.position

            let offset = val.subtract(new paper.Point(curGlobalPivot.x, curGlobalPivot.y))
            let nextShapePosition = curShapePosition.add(offset)

            if (this.paperShape.position.getDistance(nextShapePosition) <= eps) {
                return
            }

            this.paperShape.position = nextShapePosition
            this.rawObj.SetGlobalPivotPosition(val.x, val.y, 0.0)

            if (this.followCurve && isUpdatefollowCurve) {
                let followCurveShape = this.followCurve
                let length = followCurveShape.getGlobalOffsetOf(val)
                let lengthPortion = length / followCurveShape.length()

                let frameId = this.getLayer().GetCurrentFrame()
                this.shapeFollowCurveFrameState.RecordLengthRatio(frameId, lengthPortion)
            }

            if (callHandlers)
                this.valueChangeHandler.callHandlers("position", val)
        } finally {
            this.paperItem.scaling = currentScaling
            this.update(forceUpdate)
        }
    }

    set position(val: paper.Point) {
        this.setParentLocalPosition(val)
    }

    set scaling(val: paper.Point) {
        if (this.paperItem.scaling.getDistance(val) < eps)
            return

        console.log("Save scale:" + val.x + "," + val.y)
        this.rawObj.SetScale(val.x, val.y, 0)
        this.valueChangeHandler.callHandlers("scaling", val)

        this.update(true)
    }

    set rotation(val: number) {
        this.setRotation(val)
    }

    get selected(): boolean {
        return this.isSelected
    }

    set selected(val: boolean) {
        this.isSelected = val

        this.updateBoundingBox()
    }

    // Remove these functions later.
    registerValueChangeHandler(valueName: string, preProcessor: Function = null) {
        return this.valueChangeHandler.registerValueChangeHandler(valueName, preProcessor)
    }

    unregisterValueChangeHandler(valueName: string) {
        return this.valueChangeHandler.unregisterValueChangeHandler(valueName)
    }

    callHandlers(propertyName: string, val: any) {
        this.valueChangeHandler.callHandlers(propertyName, val)
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
        this.isLoadedFromFile = true
        this.update(true);
    }

    getShapeName() {
        return "UnknownShape";
    }

    getPaperJs() {
        if (paper.project)
            return paper
        return window.paper
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

    // TODO: Do we really need a store ????
    store() {
        let segments = this.getSegments()
        if (segments) {
            this.storeSegments(segments)
        }

        // Store index
        let index = this.paperItem.index
        if (index != this.rawObj.GetIndex()) { // If index changed, all the shapes in the same layer might also be changed.
            this.storeSameLayerShapeIndices()
        }

        this.updateBoundingBox()
    }

    constructor(rawObj?) {
        if (!rawObj) {
            let _this = this

            Logger.info("BaseShapeJS: Submitted execute method")
            huahuoEngine.ExecuteAfterInited(() => {

                Logger.info("BaseShapeJS: Executing raw obj creation method")
                _this.rawObj = Module.BaseShape.prototype.CreateShape(_this.getShapeName());

                _this.rawObj.SetBornFrameId(_this.getLayer().GetCurrentFrame())
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

    isSegmentSeletable() {
        return true
    }

    // Not sure how to pass getter/setter as functor.
    private getPosition() {
        return this.position
    }

    private setPosition(x: number, y: number) {
        this.position = new paper.Point(x, y)
    }

    private getScaling() {
        return this.scaling
    }

    private setScaling(x: number, y: number) {
        this.scaling = new paper.Point(x, y)
    }

    private getRotation(): number {
        return this.rotation
    }

    public setRotation(val: number, callHandlers: boolean = true, forceUpdate: boolean = true) {

        if (Math.abs(this.rawObj.GetRotation() - val) < eps)
            return

        this.rawObj.SetRotation(val)
        if (callHandlers)
            this.valueChangeHandler.callHandlers("rotation", val)
        this.update(forceUpdate)
    }

    get followCurve(): BaseShapeJS {
        let shapeObj = this.shapeFollowCurveFrameState.GetTargetShape()
        let followCurveShape = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(shapeObj)

        if (followCurveShape && !this.followCurveEventRegistered) {
            let _this = this
            followCurveShape.registerValueChangeHandler("position|scaling|rotation")(() => {
                _this.update(true)
                eventBus.triggerEvent("HHEngine", "CurveShapeTransformed")
            })

            this.followCurveEventRegistered = true
        }

        return followCurveShape
    }

    set followCurve(target: BaseShapeJS) {
        let frameId = this.getLayer().GetCurrentFrame()

        if (target != null && target.getRawShape)
            this.shapeFollowCurveFrameState.RecordTargetShape(frameId, target.getRawShape())
        else
            this.shapeFollowCurveFrameState.RecordTargetShape(frameId, null)
    }

    length() {
        return this.paperShape.length
    }

    getFollowCurve() {
        return this.followCurve
    }

    setFollowCurve(curve: BaseShapeJS) {
        if (curve != this && curve != this.followCurve) {
            this.followCurve = curve
            this.setFollowCurveLength(0.0)
        } else {
            Logger.error("Can't bind the path !")
        }
    }

    unfollowCurve() {
        this.followCurve = null
    }

    atFollowCurveStart() {
        this.setFollowCurveLength(0.0)
    }

    atFollowCurveEnd() {
        this.setFollowCurveLength(1.0)
    }

    getFollowCurveLength() {
        if (this.followCurve) {
            return this.followCurve.getGlobalOffsetOf(this.position)
        }
    }

    setFollowCurveLength(portion: number) {
        if (portion < 0.0) portion = 0.0
        if (portion > 1.0) portion = 1.0

        if (this.followCurve) {

            let totalLength = this.followCurve.length()
            let targetLength = totalLength * portion

            let curvePoint = this.followCurve.getPointAt(targetLength)
            this.position = this.followCurve.localToGlobal(curvePoint)

            let frameId = this.getLayer().GetCurrentFrame()
            this.shapeFollowCurveFrameState.RecordLengthRatio(frameId, portion)

            this.update(true)
        }
    }

    getFollowCurveLengthPotion(globalPoint: paper.Point) {
        if (this.followCurve) {
            let totalLength = this.followCurve.length()
            let currentLength = this.followCurve.getGlobalOffsetOf(globalPoint)

            return currentLength / totalLength
        }

        return -1.0
    }

    getName(): string {
        return this.name
    }

    setName(name: string) {
        this.name = name
    }

    getBornFrame() {
        return this.rawObj.GetBornFrameId() + 1 // Stored in Cpp side starts from 0 while during display starts from 1.
    }

    afterWASMReady() {
        this.shapeFollowCurveFrameState = castObject(this.rawObj.GetFrameStateByTypeName("ShapeFollowCurveFrameState"), Module["ShapeFollowCurveFrameState"])

        if (Module[this.getShapeName()]) // If the shape exists in cpp side, do the cast. Otherwise, use default BaseShape.
            this.rawObj = castObject(this.rawObj, Module[this.getShapeName()]);

        this.shapeCenterSelector = new ShapeCenterSelector(this)

        // TODO: Should move property related code to IDE, but how???
        this.propertySheet = new PropertySheet();

        let componentConfigSheet = {
            key: "inspector.BaseProperties",
            type: PropertyType.COMPONENT,
            config: {
                children: []
            }
        }

        componentConfigSheet.config.children.push({
            key: "inspector.Type",
            type: PropertyType.STRING,
            getter: this.getTypeName.bind(this)
        })

        componentConfigSheet.config.children.push({
            key: "inspector.Name",
            type: PropertyType.STRING,
            getter: this.getName.bind(this),
            setter: this.setName.bind(this)
        })

        componentConfigSheet.config.children.push({
            key: "inspector.BornFrame",
            type: PropertyType.STRING,
            getter: this.getBornFrame.bind(this),
        })

        let _this = this
        // Position
        componentConfigSheet.config.children.push({
            key: "inspector.Position",
            type: PropertyType.GROUP,
            config: {
                children: [
                    {
                        key: "inspector.FixedPosition",
                        type: PropertyType.VECTOR2,
                        getter: this.getPosition.bind(this),
                        setter: this.setPosition.bind(this),
                        registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("position"),
                        unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("position")
                        config: {
                            getKeyFrameCurves: ()=>{
                                let component = _this.getComponentByTypeName("ShapeTransformFrameState")

                                return [component.rawObj.GetVectorKeyFrameCurve(fieldName, 0), component.GetVectorKeyFrameCurve(fieldName, 1)]
                            }
                        }
                    },
                    {
                        key: "inspector.FollowPath",
                        type: PropertyType.PANEL,
                        config: {
                            children: [
                                {
                                    key: "inspector.FollowTarget",
                                    type: PropertyType.REFERENCE,
                                    getter: this.getFollowCurve.bind(this),
                                    setter: this.setFollowCurve.bind(this)
                                },
                                {
                                    key: "inspector.Unfollow",
                                    type: PropertyType.BUTTON,
                                    config: {
                                        action: this.unfollowCurve.bind(this)
                                    }
                                },
                                {
                                    key: "inspector.AtBegin",
                                    type: PropertyType.BUTTON,
                                    config: {
                                        action: this.atFollowCurveStart.bind(this)
                                    }
                                },
                                {
                                    key: "inspector.AtEnd",
                                    type: PropertyType.BUTTON,
                                    config: {
                                        action: this.atFollowCurveEnd.bind(this)
                                    }
                                },
                                {
                                    key: "inspector.AtLength",
                                    type: PropertyType.NUMBER,
                                    getter: this.getFollowCurveLength.bind(this),
                                    setter: this.setFollowCurveLength.bind(this),
                                    registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("position", this.getFollowCurveLengthPotion.bind(this)),
                                    unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("position"),
                                    config: {
                                        step: 0.01
                                    }
                                }
                            ]
                        }
                    },
                ]
            }
        });

        componentConfigSheet.config.children.push({
            key: "inspector.Scaling",
            type: PropertyType.VECTOR2,
            getter: this.getScaling.bind(this),
            setter: this.setScaling.bind(this),
            registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("scaling"),
            unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("scaling")
        })

        componentConfigSheet.config.children.push({
            key: "inspector.Rotation",
            type: PropertyType.NUMBER,
            getter: this.getRotation.bind(this),
            setter: this.setRotation.bind(this),
            registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("rotation"),
            unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("rotation")
        })

        let transformFrameStateSheet = this.getComponentConfigSheet(BASIC_COMPONENTS)
        let followCurveFrameStateSheet = this.getComponentConfigSheet("ShapeFollowCurveFrameState")

        componentConfigSheet.config.children.push({
            key: "inspector.property.keyframes",
            type: PropertyType.GROUP,
            targetObject: this,
            config: {
                children: [
                    transformFrameStateSheet,
                    followCurveFrameStateSheet
                ]
            }
        })

        this.propertySheet.addProperty(componentConfigSheet)
    }

    getComponentConfigSheet(componentName) {
        let _this = this
        if (componentName == BASIC_COMPONENTS) {
            return {
                key: "inspector." + componentName,
                type: PropertyType.KEYFRAMES,
                getter: () => {
                    let keyFrames = []
                    for (let componentName of basicComponents) {
                        let componentKeyFrames = _this.getComponentKeyFrames(componentName).bind(_this)()

                        for (let keyFrameId of componentKeyFrames) {
                            if (keyFrames.indexOf(keyFrameId) == -1) {
                                keyFrames.push(keyFrameId)
                            }
                        }
                    }

                    return keyFrames
                },
                setter: this.insertComponentKeyFrame("ShapeTransformFrameState").bind(this), // How to handle setter??
                deleter: (frameId) => {
                    for (let componentName of basicComponents) {
                        _this.deleteComponentKeyFrame(componentName).bind(_this)(frameId)
                    }
                },
                updater: (type, params) => {
                    for (let componentName of basicComponents) {
                        _this.updateComponentKeyFrame(componentName).bind(_this)(type, params)
                    }
                },
                targetObject: _this
            }
        } else {
            return {
                key: "inspector." + componentName,
                type: PropertyType.KEYFRAMES,
                getter: this.getComponentKeyFrames(componentName).bind(this),
                setter: this.insertComponentKeyFrame(componentName).bind(this),
                deleter: this.deleteComponentKeyFrame(componentName).bind(this),
                updater: this.updateComponentKeyFrame(componentName).bind(this),
                targetObject: _this
            }
        }
    }

    updateComponentKeyFrame(componentName) {
        let _this = this

        let frameStateRawObj = this.rawObj.GetFrameStateByTypeName(componentName)

        return function (type: string, params: object) {
            if (type == "ReverseKeyFrames") {
                let startFrameId = params["startFrameId"]
                let endFrameId = params["endFrameId"]

                let currentFrameId = _this.getLayer().GetCurrentFrame()
                frameStateRawObj.ReverseKeyFrame(startFrameId, endFrameId, currentFrameId);

                _this.update(true)
            }
        }
    }

    getComponentKeyFrames(componentName) {
        let frameStateRawObj = this.rawObj.GetFrameStateByTypeName(componentName)
        return function () {
            let keyFrameCount = frameStateRawObj.GetKeyFrameCount()
            let keyFrames = []
            for (let idx = 0; idx < keyFrameCount; idx++) {
                keyFrames.push(frameStateRawObj.GetKeyFrameAtIndex(idx))
            }
            return keyFrames
        }
    }

    insertComponentKeyFrame(componentName) {
        return function () {

        }
    }

    deleteComponentKeyFrame(componentName) {
        let _this = this
        return function (frameId) {
            console.log("Trying to delete keyframe:" + frameId + " from component:" + componentName)

            let component = _this.rawObj.GetFrameStateByTypeName(componentName)
            component.DeleteKeyFrame(frameId)

            _this.rawObj.SyncBornFrameIdWithComponents()

            _this.update(true)
        }
    }

    getPropertySheet() {
        if (this.selectedMeta) {
            return this.selectedMeta.propertySheet
        }

        return this.propertySheet
    }

    getMaxLengthRatio() {
        return 1.0
    }

    getCurveLength() {
        return this.paperShape.length
    }

    getRawShape() {
        return this.rawObj
    }

    isVisible() {
        return this.rawObj.IsVisible();
    }

    beforeUpdate(force: boolean = false) {
        if (this.isLoadedFromFile && !this.rawObj.IsVisible()) {
            this.selected = false
        }
    }

    hitTypeSelectable(hitType) {
        return true;
    }

    createShape() {
        this.bornStoreId = huahuoEngine.GetCurrentStoreId()

        // Dispatch the event only once, we will create paperItem for all the shapes.
        // Event for audio, we will create a placeholder shape, but might hide it in play mode.
        if (!this.paperItem)
            huahuoEngine.dispatchEvent("HHEngine", "BeforeJSShapeCreated", this)
    }

    afterCreateShape() {
        if (!this.isLoadedFromFile) {
            let paperPos = this.paperShape.position

            let localPos = this.paperShape.globalToLocal(paperPos)

            this.rawObj.SetGlobalPivotPosition(paperPos.x, paperPos.y, 0.0);
            this.rawObj.SetLocalPivotPosition(localPos.x, localPos.y, 0.0);
        }
    }

    executeAfterPaperItemReady(func) {
        if (this.paperItem) {
            func(this.paperItem)
        } else {
            this.registerValueChangeHandler("paperItemReady")(func)
        }
    }

    preparePaperItem(force: boolean = false) {
        if (!this.paperItem) {
            this.createShape()

            this.callHandlers("paperItemReady", this.paperItem)
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

        this.callHandlers("segment", null)
    }

    removeSegment(segment) {
        // Update all frames.
        this.rawObj.RemoveSegment(segment.index)

        segment.remove()

        this.updateBoundingBox()

        this.callHandlers("segment", null)
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
        /*
        let OB = localPos.x + localPos.y * Math.tan(radian)
        let OC = OB * Math.cos(radian)
        let zx = globalPos.x - OC

        let PB = localPos.y / Math.cos(radian)
        let CB = OB * Math.sin(radian)
        let PC = PB - CB

        let zy = globalPos.y - PC
         */

        // Eliminate all the divides to avoid divide zero situation
        let OC = localPos.x * Math.cos(radian) + localPos.y * Math.sin(radian)
        let zx = globalPos.x - OC

        let PC = localPos.y * Math.cos(radian) - localPos.x * Math.sin(radian)
        let zy = globalPos.y - PC

        return new paper.Point(zx, zy)
    }

    get localPivotPosition() {
        return this.rawObj.GetLocalPivotPosition()
    }

    updatePositionAndRotation() {
        // Reset the rotation.
        this.paperItem.rotation = this.rotation;

        this.paperItem.scaling = new paper.Point(1.0, 1.0)

        this.paperItem.position = new paper.Point(0.0, 0.0)
        // The coordinate should have been aligned now.
        let globalPivotPosition = this.pivotPosition
        let localPivotPosition = this.rawObj.GetLocalPivotPosition()

        let radian = this.rotation / 180 * Math.PI

        let shapeZero = this.backCalculateZeroPoint(localPivotPosition, globalPivotPosition, -radian)

        let offset = this.paperShape.localToParent(new paper.Point(0, 0)) // As position is already (0,0). The global position of (0,0) indicates the center of the bounding rect.

        let newPosition = shapeZero.subtract(offset)
        this.paperItem.position = newPosition
    }

    afterUpdate(force: boolean = false) {
        this.applySegments()

        // Reset the rotation.
        this.paperItem.rotation = this.rotation;

        if (this._isUpdatePos) {
            this.updatePositionAndRotation()
        }

        let scaling = this.rawObj.GetScale()
        this.paperItem.scaling = new paper.Point(scaling.x, scaling.y)

        // Adjust index
        if (this.paperItem.index != this.rawObj.GetIndex() && this.paperItem.index > 0) {
            let parent = this.paperItem.parent
            if (parent) {
                parent.insertChild(this.rawObj.GetIndex(), this.paperItem)
            }
        }

        // Execute after update of all components
        for (let component of this.customComponents) {
            if (component.isComponentActive()) {
                component.afterUpdate(force)
            }
        }
    }

    hide() {
        this.paperItem.visible = false
        this.selected = false
        this.updateBoundingBox()

        this.callHandlers("shapeHidden", null)
    }

    update(force: boolean = false) {
        let currentFrame = this.getLayer().GetCurrentFrame()
        // if (force || currentFrame != this.lastRenderFrame) { // TODO: Because of event graph, we might still need to update here.
        {
            totallyUpdated++
            // console.log("Totally updated:" + totallyUpdated)

            this.beforeUpdate(true)
            this.preparePaperItem(true)

            if (!this.rawObj.IsVisible()) {
                this.paperItem.visible = false
                this.selected = false

                // Execute after update of all components
                for (let component of this.customComponents) {
                    component.setInvisible()
                }

            } else {
                this.paperItem.visible = true
                this.afterUpdate(true)
            }

            this.updateBoundingBox()

            this.lastRenderFrame = currentFrame
        }
    }

    getPaperPoint(engineV3Point) {
        return new paper.Point(engineV3Point.x, engineV3Point.y)
    }

    remove() {
        // TODO: TODO
        huahuoEngine.DestroyShape(this.rawObj)
        this.removePaperObj()

        this.callHandlers("shapeRemoved", null)
    }

    removePaperObj() {
        this.paperItem.remove()
        if (this.boundingBoxGroup)
            this.boundingBoxGroup.remove()
        this.selected = false

        this.paperItem = null
        this.shapeCenterSelector.selected = false

        for (let component of this.customComponents) {
            component.cleanUp()
        }
    }

    detachFromCurrentLayer() {
        this.getLayer().RemoveShape(this.rawObj)
    }

    // Pass in a set to avoid creation of the set multiple times.
    getReferencedShapes(set: Set<BaseShapeJS>) {

        if (this.followCurve) {
            set.add(this.followCurve)
        }

        // Get all referenced shapes
        for (let component of this.customComponents) {
            component.getReferencedShapes(set)
        }
    }
}

export {BaseShapeJS}