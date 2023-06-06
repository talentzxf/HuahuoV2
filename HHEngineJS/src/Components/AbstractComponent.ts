import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
import {propertySheetFactory} from "./PropertySheetBuilderFactory"
import {CustomFieldConfig, PropertyConfig, PropertyType} from "hhcommoncomponents";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {ComponentConfig} from "./ComponentConfig";
import {defaultVariableProcessor} from "./VariableHandlers/DefaultVariableProcessor";
import {shapeArrayHandler} from "./VariableHandlers/ShapeArrayHandler";
import {colorStopArrayHandler} from "./VariableHandlers/ColorArrayProcessor";
import {subComponentArrayHandler} from "./VariableHandlers/SubComponentArrayHandler";
import {customFieldVariableHandler} from "./VariableHandlers/CustomFieldVariableHandler";
import {huahuoEngine} from "../EngineAPI";


// Key is: className#fieldName
// Value is the constructor of the divContent generator
let customFieldContentDivGeneratorMap: Map<string, Function> = new Map()

function registerCustomFieldContentDivGeneratorConstructor(className: string, fieldName: string, constructor: Function) {
    let fieldFullName = className + "#" + fieldName
    customFieldContentDivGeneratorMap.set(fieldFullName, constructor)
}

function getCustomFieldContentDivGeneratorConstructor(className: string, fieldName: string): Function {
    let fieldFullName = className + "#" + fieldName
    return customFieldContentDivGeneratorMap.get(fieldFullName)
}

const metaDataKey = Symbol("objectProperties")
declare var Module: any;

function getProperties(target): object[] {
    let properties: object[] = Reflect.getMetadata(metaDataKey, target)
    if (!properties) {
        properties = new Array<PropertyDef>()
        Reflect.defineMetadata(metaDataKey, properties, target)
    }

    return properties
}

function PropertyValue(category: PropertyCategory, initValue = null, config?: PropertyConfig, hide: boolean = false) {
    return function (target: object, propertyKey: string) {
        let properties = getProperties(target)
        let propertyEntry: PropertyDef = {
            key: propertyKey,
            type: category,
            initValue: initValue,
            config: config,
            hide: hide
        }
        properties.push(propertyEntry)
    }
}

function Component(componentConfig?: ComponentConfig) {
    return function (ctor) {
        clzObjectFactory.RegisterClass(ctor.name, ctor)

        clzObjectFactory.RegisterComponent(ctor.name, componentConfig)
    }
}

declare function castObject(obj: any, clz: any): any;

class AbstractComponent {

    isBuiltIn: boolean = false

    rawObj: any;
    baseShape: BaseShapeJS;
    propertySheetInited: boolean = false;
    shapeArrayFieldNames: Set<string> = new Set<string>()

    // If this component belongs to a mirage shape, it should also be mirage.
    isMirage: boolean = false

    // Not sure why, but if we define variable here, will enter infinite loop.
    // @PropertyValue(PropertyCategory.boolean, false)
    // isActive

    protected valueChangeHandler: ValueChangeHandler = new ValueChangeHandler()

    registerValueChangeHandler(valueNameString: string, callbackFunc: Function){
        this.valueChangeHandler.registerValueChangeHandler(valueNameString)(callbackFunc)
    }

    callHandlers(propertyName: string, val: any) {
        this.valueChangeHandler.callHandlers(propertyName, val)
    }

    constructor(rawObj?, isMirage = false) {
        this.isMirage = isMirage

        let cppClzName = clzObjectFactory.getCppClassName(this.constructor.name)

        if (!rawObj) {
            rawObj = Module[cppClzName].prototype.CreateComponent(cppClzName)
        }

        this.rawObj = castObject(rawObj, Module[cppClzName])

        if (!this.rawObj.IsFieldRegistered("isActive")) {
            this.rawObj.RegisterBooleanValue("isActive", true)
        }

        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)
        if (properties) {
            properties.forEach(propertyEntry => {
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
            })
        }

        this.rawObj.SetTypeName(this.constructor.name)
    }

    setBaseShape(baseShape: BaseShapeJS) {
        this.baseShape = baseShape
        this.enableComponent() // Enable the component after baseShape is set.
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
    onComponentEnabled(){

    }

    // Life cycle function, call back when the component is disabled.
    onComponentDisabled(){

    }

    disableComponent() {
        this.rawObj.SetBooleanValue("isActive", false)

        this.onComponentDisabled()

        if(this.baseShape)
            this.baseShape.update(true)


    }

    enableComponent() {
        this.rawObj.SetBooleanValue("isActive", true)
        this.onComponentEnabled()

        // Something might be changed, so we need to refresh the shape.
        // But if the shape or the paper shape has not been created yet, do not refresh it.
        // TODO: if there're a lot of components, how to avoid update again and again?
        if(this.baseShape && this.baseShape.paperShape)
            this.baseShape.update(true)
    }

    getPropertySheet() {
        let _this = this
        let componentConfigSheet = {
            key: this.getTypeName(),
            type: PropertyType.COMPONENT,
            targetObject: this.baseShape,
            config: {
                children: [],
                enabler: () => {
                    _this.enableComponent()
                },
                disabler: () => {
                    _this.disableComponent()
                },
                isActive: () => {
                    return _this.isComponentActive()
                }
            }
        }

        if(!this.isBuiltIn){
            componentConfigSheet.config["deleter"] = ()=>{
                huahuoEngine.dispatchEvent("HHIDE", "DeleteComponent", this)
            }
        }

        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)
        if (properties != null) {
            for (let propertyMeta of properties) {
                if (propertyMeta.type == PropertyCategory.customField) {
                    if (propertyMeta.config == null || propertyMeta.config["contentDivGenerator"] == null) {

                        propertyMeta = {...propertyMeta} // Clone it to avoid affecting other objects. Shallow copy should be enough.

                        let divGeneratorConstructor = getCustomFieldContentDivGeneratorConstructor(this.constructor.name, propertyMeta.key)

                        // @ts-ignore
                        let contentDivGenerator = new divGeneratorConstructor(this)
                        propertyMeta.config = {
                            fieldName: propertyMeta["key"],
                            contentDivGenerator: contentDivGenerator
                        } as CustomFieldConfig
                    }
                }
                let propertySheetEntry = propertySheetFactory.createEntry(this, propertyMeta, this.valueChangeHandler)
                if (propertySheetEntry != null) {
                    componentConfigSheet.config.children.push(propertySheetEntry)
                }
            }
        }

        let keyFramePropertySheet = propertySheetFactory.createEntryByNameAndCategory("keyframes", PropertyCategory.keyframeArray)

        keyFramePropertySheet["getter"] = this.getKeyFrames.bind(this)
        keyFramePropertySheet["targetObject"] = this.baseShape
        keyFramePropertySheet["deleter"] = this.baseShape.deleteComponentKeyFrame(this.getTypeName()).bind(this.baseShape)

        componentConfigSheet.config.children.push(keyFramePropertySheet)

        return componentConfigSheet
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

    initPropertySheet(propertySheet) {
        if (!this.propertySheetInited) {
            this.propertySheetInited = true

            let myPropertySheet = this.getPropertySheet()
            myPropertySheet["rawObjPtr"] = this.rawObj.ptr
            if (myPropertySheet)
                propertySheet.addProperty(myPropertySheet)
        }
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

    getKeyFrameCurve(fieldName){
        return this.rawObj.GetFloatKeyFrameCurve(fieldName)
    }

    getVector2KeyFrameCurves(fieldName){
        return [this.rawObj.GetVectorKeyFrameCurve(fieldName, 0), this.rawObj.GetVectorKeyFrameCurve(fieldName, 1)]
    }

    detachFromCurrentShape() {
        this.baseShape.removeComponent(this)
    }
}


export {AbstractComponent, PropertyValue, Component, registerCustomFieldContentDivGeneratorConstructor}