import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
import {EventEmitter, getFieldNameFromGetterName, PropertyConfig} from "hhcommoncomponents";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {ComponentConfig} from "./ComponentConfig";
import {defaultVariableProcessor} from "./VariableHandlers/DefaultVariableProcessor";
import {shapeArrayHandler} from "./VariableHandlers/ShapeArrayHandler";
import {colorStopArrayHandler} from "./VariableHandlers/ColorArrayProcessor";
import {subComponentArrayHandler} from "./VariableHandlers/SubComponentArrayHandler";
import {customFieldVariableHandler} from "./VariableHandlers/CustomFieldVariableHandler";
import {ComponentActions} from "../EventGraph/GraphActions";
import {addComponentProperties} from "../EventGraph/LGraphSetup";
import {ComponentActor} from "./ComponentActor";
import {capitalizeFirstLetter} from "hhcommoncomponents";

const metaDataKey = Symbol("objectProperties")
declare var Module: any;

function getProperties(target): Array<PropertyDef> {
    let properties: Array<PropertyDef> = Reflect.getMetadata(metaDataKey, target)
    if (!properties) {
        properties = new Array<PropertyDef>()
        Reflect.defineMetadata(metaDataKey, properties, target)
    }

    return properties
}

function PropertyValue(category: PropertyCategory, initValue = null, config: PropertyConfig = null, hide: boolean = false, singleLine = false) {
    return function (target: object, propertyKey: string, descriptor: PropertyDescriptor = null) {
        let properties = getProperties(target)

        let propertyEntry: PropertyDef = {
            key: propertyKey,
            type: category,
            initValue: initValue,
            config: config,
            hide: hide,
            singleLine: singleLine
        }

        if (descriptor != null && typeof descriptor.value === "function") {
            propertyEntry["key"] = getFieldNameFromGetterName(propertyKey)
            propertyEntry["isFunction"] = true
        }

        properties.push(propertyEntry)
        addComponentProperties(target.constructor.name, properties)
    }
}

function Component(componentConfig?: ComponentConfig) {
    return function (ctor) {
        clzObjectFactory.RegisterClass(ctor.name, ctor)

        clzObjectFactory.RegisterComponent(ctor.name, componentConfig)
    }
}

declare function castObject(obj: any, clz: any): any;

abstract class AbstractComponent extends EventEmitter {

    static getMetaDataKey() {
        return metaDataKey
    }

    canBeDisabled(){
        return true
    }

    private actor: ComponentActor = new ComponentActor(this)

    componentActions: ComponentActions

    isBuiltIn: boolean = false

    rawObj: any;
    baseShape: BaseShapeJS;
    shapeArrayFieldNames: Set<string> = new Set<string>()

    // If this component belongs to a mirage shape, it should also be mirage.
    isMirage: boolean = false

    observedFields: Map<string, Set<string>> = new Map

    // Not sure why, but if we define variable here, will enter infinite loop.
    // @PropertyValue(PropertyCategory.boolean, false)
    // isActive
    protected valueChangeHandler: ValueChangeHandler = new ValueChangeHandler()

    getActor(){

        return this.actor
    }

    getRawObject() {
        return this.rawObj
    }

    registerValueChangeHandler(valueNameString: string, callbackFunc: Function): number {
        return this.valueChangeHandler.registerValueChangeHandler(valueNameString)(callbackFunc)
    }

    unregisterValueChangeHandlerFromAllValues(id) {
        this.valueChangeHandler.unregisterValueChangeHandlerFromAllValues(id)
    }

    callHandlers(propertyName: string, val: any) {
        this.valueChangeHandler.callHandlers(propertyName, val)

        if (this.observedFields.has(propertyName)) {
            for (let fieldName of this.observedFields.get(propertyName)) {
                let newResult = null

                if (this.hasOwnProperty(fieldName)) {
                    newResult = this[fieldName]
                } else {
                    let currentGetter = "get" + capitalizeFirstLetter(fieldName)
                    if (this[currentGetter] && typeof this[currentGetter] == "function") {
                        newResult = this[currentGetter]()
                    }
                }

                this.valueChangeHandler.callHandlers(fieldName, newResult)
            }
        }
    }

    addProperty(propertyEntry: PropertyDef, needAppendInMeta: boolean = false) {
        // @ts-ignore
        let observedFields = propertyEntry?.config?.observedFields
        if (observedFields != null) {
            for (let field of observedFields) {
                if (!this.observedFields.has(field)) {
                    this.observedFields.set(field, new Set<string>())
                }

                this.observedFields.get(field).add(propertyEntry.key)
            }
        }

        if (propertyEntry.hasOwnProperty("isFunction") && propertyEntry["isFunction"] == true) {
            return
        }

        if (needAppendInMeta) {
            let currentProperties = getProperties(this)
            currentProperties.push(propertyEntry)

            this.rawObj.AddAdditionalFieldDef(propertyEntry.key, JSON.stringify(propertyEntry))
        }

        if (propertyEntry.type == PropertyCategory.shapeArray) {
            shapeArrayHandler.handleEntry(this, propertyEntry)
        } else if (propertyEntry.type == PropertyCategory.colorStopArray) {
            colorStopArrayHandler.handleEntry(this, propertyEntry)
        } else if (propertyEntry.type == PropertyCategory.subcomponentArray) {
            // Only Components inherits GroupComponent can have subComponentArray. Cause SubComponentArray itself is a component.
            //@ts-ignore
            subComponentArrayHandler.handleEntry(this, propertyEntry)
        } else if (propertyEntry.type == PropertyCategory.customField) {
            customFieldVariableHandler.handleEntry(this, propertyEntry)
        } else {
            defaultVariableProcessor.handleEntry(this, propertyEntry)
        }
    }

    constructor(rawObj?, isMirage = false) {
        super()

        this.isMirage = isMirage

        let cppClzName = clzObjectFactory.getCppClassName(this.constructor.name)

        if (!rawObj) {
            rawObj = Module[cppClzName].prototype.CreateComponent(cppClzName)
        }

        this.rawObj = castObject(rawObj, Module[cppClzName])

        let frameStateCount = this.rawObj.GetFrameStateCount()
        for (let frameStateIdx = 0; frameStateIdx < frameStateCount; frameStateIdx++) {
            let frameStateRawObj = this.rawObj.GetFrameStateAtIdx(frameStateIdx)
            let frameStateName = frameStateRawObj.GetName()
            let frameStateString = this.rawObj.GetAdditionalFieldDef(frameStateName)

            if (frameStateString != null && frameStateString.length > 0) {
                let fieldDef = JSON.parse(frameStateString)
                const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)
                properties.push(fieldDef)
            }
        }

        if (!this.rawObj.IsFieldRegistered("isActive")) {
            this.rawObj.RegisterBooleanValue("isActive", true)
        }

        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)
        if (properties) {
            properties.forEach(propertyEntry => {
                this.addProperty(propertyEntry)
            })
        }

        this.rawObj.SetTypeName(this.constructor.name)

        this.componentActions = new ComponentActions(this)
    }

    getActionDefs() {
        return this.componentActions.getActionDefs()
    }

    setBaseShape(baseShape: BaseShapeJS) {
        this.baseShape = baseShape

        // I don't like this, but baseShape will be null in some cases???  Have to manual fix here....
        this.rawObj.SetBaseShape(baseShape.getRawObject())
    }

    afterUpdate(force: boolean = false) {
    }

    getCurrentFrameId() {
        return this.baseShape.getLayer().GetCurrentFrame()
    }

    getTypeName() {
        return this.rawObj.GetTypeName()
    }

    store() {

    }

    isComponentActive() {
        return this.rawObj.GetBooleanValue("isActive")
    }

    // Life cycle function, call back when the component is enabled.
    onComponentEnabled() {

    }

    // Life cycle function, call back when the component is disabled.
    onComponentDisabled() {
        this.baseShape.getActor().RemoveActionInvoker(this)
    }

    disableComponent() {
        this.rawObj.SetBooleanValue("isActive", false)
        this.onComponentDisabled()

        if (this.baseShape)
            this.baseShape.update(true)
    }

    enableComponent() {
        this.rawObj.SetBooleanValue("isActive", true)

        this.onComponentEnabled()

        // Something might be changed, so we need to refresh the shape.
        // But if the shape or the paper shape has not been created yet, do not refresh it.
        // TODO: if there're a lot of components, how to avoid update again and again?
        if (this.baseShape && this.baseShape.paperShape && !this.baseShape.isLoadingComponents)
            this.baseShape.update(true)
    }

    getKeyFrames() {
        let keyFrameCount = this.rawObj.GetKeyFrameCount()
        let keyFrames = []
        for (let idx = 0; idx < keyFrameCount; idx++) {
            keyFrames.push(this.rawObj.GetKeyFrameAtIndex(idx))
        }
        return keyFrames
    }

    insertKeyFrames(val) {

    }

    cleanUp() {

    }

    setInvisible() {

    }

    // Pass in a set to avoid creation of the set multiple times.
    getReferencedShapes(set: Set<BaseShapeJS>) {

        // Put all shapeArray values in the set.
        for (let fieldName of this.shapeArrayFieldNames) {
            for (let shape of this[fieldName]) {
                set.add(shape)
            }
        }
    }

    getKeyFrameCurve(fieldName) {
        return this.rawObj.GetFloatKeyFrameCurve(fieldName)
    }

    getVector2KeyFrameCurves(fieldName) {
        return [this.rawObj.GetVectorKeyFrameCurve(fieldName, 0), this.rawObj.GetVectorKeyFrameCurve(fieldName, 1)]
    }

    detachFromCurrentShape() {
        this.baseShape.removeComponent(this)
    }

    reset() {
        this.actor.reset()
    }

    // Callback from IDE when the component is mounted.
    onMounted() {

    }

    // Callback from IDE when the component is dismounted from the shape.
    onDismounted() {

    }
}


export {AbstractComponent, PropertyValue, Component, getProperties}