import {huahuoEngine} from "../EngineAPI";
import {GetObjPtr, IsValidWrappedObject, Logger, PropertySheet, PropertyType} from "hhcommoncomponents";
import * as paper from "paper";
import {ValueChangeHandler} from "./ValueChangeHandler";
import {AbstractComponent} from "../Components/AbstractComponent";
import {BaseShapeActor} from "../EventGraph/BaseShapeActor";
import {clzObjectFactory} from "../CppClassObjectFactory";

let BASIC_COMPONENTS = "BasicComponents"

let basicComponents = ["ShapeTransformComponent", "ShapeSegmentFrameState"]

declare function castObject(obj: any, clz: any): any;

declare var Module: any;

const eps: number = 0.001

let totallyUpdated: number = 0

abstract class BaseShapeJS {
    protected rawObj: any = null;
    protected paperItem: paper.Item
    protected isSelected = false


    // If this shape is not loaded from (i.e. generated on the fly), we might need to record some information during creation. like it's local pivot position.
    // Or else, just read these information from the file.
    protected isLoadedFromFile: boolean = false;

    // This is used for Editor only to set properties.
    protected propertySheet: PropertySheet

    private parent: BaseShapeJS = null

    private bornStoreId: string;

    private valueChangeHandler: ValueChangeHandler = new ValueChangeHandler()

    // From ptr to the component.
    private customComponentMap: Map<number, AbstractComponent> = new Map<number, AbstractComponent>()

    private _isMirage: boolean = false

    private lastRenderFrame = -1

    // Purpose of actor is to store temporary results during system running. All the status in the actor won't be persisted.
    private actor: BaseShapeActor = new BaseShapeActor(this)

    private get customComponents() {
        return this.customComponentMap.values();
    }

    public getActor():BaseShapeActor {
        return this.actor
    }

    set isTransformationPermanent(isPermanent: boolean) {
        this.rawObj.SetRecordTransformationOfKeyFrame(isPermanent)
    }

    set isMirage(val: boolean) {
        this._isMirage = val
    }

    get isMirage() {
        return this._isMirage
    }

    get belongStoreId(): string {
        return this.rawObj.GetStoreId()
    }

    get typename(): string {
        return this.rawObj.GetTypeName()
    }

    get pivotPosition(): paper.Point {
        if (this.actor.isPositionValid)
            return this.actor.position
        return this.rawObj.GetGlobalPivotPosition()
    }

    set pivotPosition(centerPosition: paper.Point) {
        let currentScaling = this.paperItem.scaling

        this.paperItem.scaling = new paper.Point(1.0, 1.0)

        let localCenterPos = this.paperItem.globalToLocal(centerPosition)
        this.rawObj.SetGlobalPivotPosition(centerPosition.x, centerPosition.y, 0.0)
        this.rawObj.SetLocalPivotPosition(localCenterPos.x, localCenterPos.y, 0.0)

        this.paperItem.scaling = currentScaling
    }

    saveAsKeyFrame() {
        if (this.rawObj)
            this.rawObj.SaveAsKeyFrame()
    }

    componentValueChangeHandlers: Array<Function> = new Array()

    addComponent(component: AbstractComponent, persistentTheComponent: boolean = true) {
        if (this.customComponentMap.has(GetObjPtr(component))) {
            return
        }

        if (persistentTheComponent) {
            this.rawObj.AddFrameState(component.rawObj)
        }

        this.customComponentMap.set(GetObjPtr(component), component)
        component.setBaseShape(this)

        let _this = this
        component.registerValueChangeHandler("*", () => {
            for (let func of _this.componentValueChangeHandlers) {
                func()
            }
        })
    }

    // If allowNull, performance will be a little better
    getComponents(allowNulls = false) {

        if (allowNulls)
            return this.customComponents

        let components = []
        for (let component of this.customComponents) {
            if (component != null)
                components.push(component)
        }

        return components
    }

    getComponentCountByTypeName(typeName) {
        let returnCount = 0
        for (let component of this.customComponents) {
            if (component == null)
                continue

            if (component.getTypeName() == typeName) {
                returnCount++
            }
        }
        return returnCount
    }

    getComponentByTypeName(typeName) {
        for (let component of this.customComponents) {
            if (component == null)
                continue;
            if (component.getTypeName() == typeName) {
                return component
            }
        }
        return null
    }

    public getPointAt(offset): paper.Point {
        return this.paperShape.getPointAt(offset)
    }

    public getBornStoreId(): string {
        return this.bornStoreId
    }

    public setBornStoreId(val: string) {
        this.bornStoreId = val
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

    getBounds() {
        return this.bounds;
    }

    get bounds(): paper.Rectangle {
        if (this.paperItem == null)
            return null

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
        if (this.actor.isRotationValid)
            return this.actor.rotation
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

        let newShapeObj = huahuoEngine.getActivePlayer().getLayerShapes(shapeLayer).get(GetObjPtr(newRawObj))
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
    setParentLocalPosition(val: paper.Point, callHandlers: boolean = true, forceUpdate: boolean = true) {
        let currentScaling = this.scaling

        try {
            if (this.paperItem != null) {
                this.paperItem.scaling = new paper.Point(1.0, 1.0)

                let curGlobalPivot = this.rawObj.GetGlobalPivotPosition()
                let curShapePosition = this.paperShape.position

                let offset = val.subtract(new paper.Point(curGlobalPivot.x, curGlobalPivot.y))
                let nextShapePosition = curShapePosition.add(offset)

                if (this.paperShape.position.getDistance(nextShapePosition) <= eps) {
                    return
                }

                this.paperShape.position = nextShapePosition
            }

            this.rawObj.SetGlobalPivotPosition(val.x, val.y, 0.0)
            if (callHandlers)
                this.valueChangeHandler.callHandlers("position", val)
        } finally {
            if (this.paperItem)
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
    }

    registerComponentValueChangeHandler(func) {
        this.componentValueChangeHandlers.push(func)
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

    // TODO: Do we really need a store ????
    store() {
        // Store index
        let index = this.paperItem.index
        if (index != this.rawObj.GetIndex()) { // If index changed, all the shapes in the same layer might also be changed.
            this.storeSameLayerShapeIndices()
        }
    }

    constructor(rawObj?) {
        if (!rawObj) {
            let _this = this

            Logger.info("BaseShapeJS: Submitted execute method")
            huahuoEngine.ExecuteAfterInited(() => {
                // layer is null when the shape is newly created, it will be set later!!
                _this.rawObj = Module.BaseShape.prototype.CreateShape(_this.getShapeName());

                let bornFrameId = huahuoEngine.GetCurrentLayer().GetCurrentFrame()
                if (IsValidWrappedObject(_this.getLayer())) {
                    bornFrameId = _this.getLayer().GetCurrentFrame()
                }

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

    // Should only be called from HHIDE when creating new shapes.
    // As the addComponent is proxied in Editor.
    // For player, all components are loaded from file.
    initShapeFromEditor() {

    }

    isSelectable() {
        return true
    }

    isSegmentSeletable() {
        return true
    }

    // Not sure how to pass getter/setter as functor.
    public getPosition(needAction: boolean = true) {
        if (needAction)
            return this.position

        let rawObjPosition = this.rawObj.GetGlobalPivotPosition()
        return new paper.Point(rawObjPosition.x, rawObjPosition.y)

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

    public getRotation(needAction: boolean = true): number {
        if (needAction)
            return this.rotation

        return this.rawObj.GetRotation()
    }

    public setRotation(val: number, callHandlers: boolean = true, forceUpdate: boolean = true) {

        if (Math.abs(this.rawObj.GetRotation() - val) < eps)
            return

        this.rawObj.SetRotation(val)
        if (callHandlers)
            this.valueChangeHandler.callHandlers("rotation", val)
        this.update(forceUpdate)
    }

    length() {
        return this.paperShape.length
    }

    getName(): string {
        return this.name
    }

    setName(name: string) {
        this.name = name

        this.valueChangeHandler.callHandlers("name", this.name)
    }

    getBornFrame() {
        return this.rawObj.GetBornFrameId() + 1 // Stored in Cpp side starts from 0 while during display starts from 1.
    }

    afterWASMReady() {
        if (Module[this.getShapeName()]) // If the shape exists in cpp side, do the cast. Otherwise, use default BaseShape.
            this.rawObj = castObject(this.rawObj, Module[this.getShapeName()]);

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
            key: "inspector.BornFrame",
            type: PropertyType.STRING,
            getter: this.getBornFrame.bind(this),
        })

        componentConfigSheet.config.children.push({
            key: "inspector.Name",
            type: PropertyType.STRING,
            getter: this.getName.bind(this),
            setter: this.setName.bind(this),
            registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("name"),
            unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("name"),
            maxLength: 20
        })

        let _this = this
        componentConfigSheet.config.children.push({
            key: "inspector.Rotation",
            type: PropertyType.NUMBER,
            getter: this.getRotation.bind(this),
            setter: this.setRotation.bind(this),
            registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("rotation"),
            unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("rotation")
        })

        // Position
        componentConfigSheet.config.children.push({
            key: "inspector.Position",
            type: PropertyType.VECTOR2,
            getter: this.getPosition.bind(this),
            setter: this.setPosition.bind(this),
            registerValueChangeFunc: this.valueChangeHandler.registerValueChangeHandler("position"),
            unregisterValueChangeFunc: this.valueChangeHandler.unregisterValueChangeHandler("position"),
            config: {
                getKeyFrameCurves: () => {
                    let transformCompoennt = _this.rawObj.GetTransform()

                    return [transformCompoennt.GetVectorKeyFrameCurve("globalPivotPosition", 0),
                        transformCompoennt.GetVectorKeyFrameCurve("globalPivotPosition", 1)]
                }
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

        let transformFrameStateSheet = this.getComponentKeyFramesPropertyConfig(BASIC_COMPONENTS)
        componentConfigSheet.config.children.push({
            key: "inspector.property.keyframes",
            type: PropertyType.GROUP,
            targetObject: this,
            singleLine: true,
            config: {
                children: [
                    transformFrameStateSheet,
                ]
            }
        })

        this.propertySheet.addProperty(componentConfigSheet)
    }

    getComponentKeyFramesPropertyConfig(componentName): object {
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
                setter: this.insertComponentKeyFrame("ShapeTransformComponent").bind(this), // How to handle setter??
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

    getComponentByRawObj(componentRawObj) {
        for (let component of this.customComponents) {
            if (component != null && GetObjPtr(component) == GetObjPtr(componentRawObj))
                return component
        }
        return null
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

    getRawObject() {
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

            if (!this.isMirage) {
                this.rawObj.SetGlobalPivotPosition(paperPos.x, paperPos.y, 0.0);
                this.rawObj.SetLocalPivotPosition(localPos.x, localPos.y, 0.0);
            }
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

        this.LoadComponents()
    }

    isLoadingComponents: Boolean = false

    LoadComponents() {
        this.isLoadingComponents = true
        // Create all the component wrapper in the JS side.
        let componentCount = this.rawObj.GetFrameStateCount()
        for (let idx = 0; idx < componentCount; idx++) {
            let componentRawObj = this.rawObj.GetFrameState(idx)

            if (!this.customComponentMap.has(GetObjPtr(componentRawObj))) {
                let component = null

                let componentConstructor = clzObjectFactory.GetClassConstructor(componentRawObj.GetTypeName())
                if (componentConstructor) {
                    component = new componentConstructor(componentRawObj, this.isMirage)
                    // The component has already been persistented, no need to persistent again.
                    this.addComponent(component, false)
                }

                this.customComponentMap.set(GetObjPtr(componentRawObj), component)
            }
        }

        this.isLoadingComponents = false
    }

    getTypeName() {
        return this.rawObj.GetTypeName()
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
        // Reset the rotation.
        this.paperItem.rotation = this.rotation;

        this.updatePositionAndRotation()

        let scaling = this.rawObj.GetScale()
        this.paperItem.scaling = new paper.Point(scaling.x, scaling.y)

        // Adjust index
        if (this.paperItem.index != this.rawObj.GetIndex() && this.paperItem.index > 0) {
            let parent = this.paperItem.parent
            if (parent) {
                parent.insertChild(this.rawObj.GetIndex(), this.paperItem)
            }
        }
    }

    hide() {
        this.paperItem.visible = false
        this.selected = false

        this.callHandlers("shapeHidden", null)
    }

    update(force: boolean = false) {
        let currentFrame = this.getLayer().GetCurrentFrame()
        if (currentFrame == this.bornFrameId) {
            this.callHandlers("onBornFrame", null)
        }

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
                    if (component == null)
                        continue

                    component.setInvisible()
                }

            } else {
                this.paperItem.visible = true
                this.afterUpdate(true)

                // Execute after update of all components
                for (let component of this.customComponents) {
                    if (component == null)
                        continue
                    if (component.isComponentActive()) {
                        component.afterUpdate(force)
                    }
                }
            }

            this.lastRenderFrame = currentFrame
        }
    }

    getPaperPoint(engineV3Point) {
        return new paper.Point(engineV3Point.x, engineV3Point.y)
    }

    isValid() {
        return this.isRemoved == false
    }

    isRemoved: boolean = false

    remove() {
        // TODO: TODO
        // huahuoEngine.DestroyShape(this.rawObj)

        this.detachFromCurrentLayer()
        this.removePaperObj()
        this.isRemoved = true

        this.callHandlers("shapeRemoved", null)
    }

    removePaperObj() {
        this.paperItem.remove()
        this.selected = false

        this.paperItem = null
        for (let component of this.customComponents) {
            if (component == null)
                continue

            component.cleanUp()
        }
    }

    detachFromCurrentLayer() {
        this.getLayer().RemoveShape(this.rawObj)
    }

    // Pass in a set to avoid creation of the set multiple times.
    getReferencedShapes(set: Set<BaseShapeJS>) {
        // Get all referenced shapes
        for (let component of this.customComponents) {
            if (component == null)
                continue

            component.getReferencedShapes(set)
        }
    }

    removeComponent(component: AbstractComponent) {
        this.customComponentMap.delete(GetObjPtr(component))
        component.rawObj.DetachFromCurrentShape()


        let properties = this.getPropertySheet().getProperties()

        let _this = this

        properties = properties.filter(function (entry) {
            if (entry.hasOwnProperty("rawObjPtr") && entry["rawObjPtr"] == GetObjPtr(component)) {
                return false;
            }

            return true;
        })

        this.getPropertySheet().setProperties(properties)
    }

    isResetting: boolean = false

    reset() {
        this.isResetting = true
        this.getActor().reset()
        for (let component of this.customComponents) {
            if (component != null)
                component.reset()
        }

        this.update()
        this.isResetting = false
    }
}

export {BaseShapeJS, eps}