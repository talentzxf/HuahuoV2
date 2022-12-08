import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
import {propertySheetFactory} from "./PropertySheetBuilderFactory"
import {PropertyConfig, PropertyType} from "hhcommoncomponents";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {ComponentConfig} from "./ComponentConfig";
import {interpolateVariableProcessor} from "./VariableHandlers/InterpolateVariableProcessor";
import {shapeArrayHandler} from "./VariableHandlers/ShapeArrayHandler";
import {colorStopArrayHandler} from "./VariableHandlers/ColorArrayProcessor";

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

function PropertyValue(category: PropertyCategory, initValue = null, config?: PropertyConfig) {
    return function (target: object, propertyKey: string) {
        let properties = getProperties(target)
        let propertyEntry: PropertyDef = {
            key: propertyKey,
            type: category,
            initValue: initValue,
            config: config
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
    rawObj: any;
    baseShape: BaseShapeJS;
    propertySheetInited: boolean = false;
    shapeArrayFieldNames: Set<string> = new Set<string>()

    private valueChangeHandler: ValueChangeHandler = new ValueChangeHandler()

    callHandlers(propertyName: string, val: any) {
        this.valueChangeHandler.callHandlers(propertyName, val)
    }

    constructor(rawObj?) {
        if (rawObj) {
            this.rawObj = castObject(rawObj, Module["CustomComponent"])
        } else {
            this.rawObj = Module["CustomComponent"].prototype.CreateComponent()
        }

        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)
        if (properties) {
            properties.forEach(propertyEntry => {
                if (propertyEntry.type == PropertyCategory.interpolateFloat
                    || propertyEntry.type == PropertyCategory.interpolateColor
                    || propertyEntry.type == PropertyCategory.interpolateVector2
                    || propertyEntry.type == PropertyCategory.interpolateVector3) {
                    interpolateVariableProcessor.handleEntry(this, propertyEntry)
                } else if (propertyEntry.type == PropertyCategory.shapeArray) {
                    shapeArrayHandler.handleEntry(this, propertyEntry)
                } else if (propertyEntry.type == PropertyCategory.colorStopArray) {
                    colorStopArrayHandler.handleEntry(this, propertyEntry)
                }
            })
        }

        this.rawObj.SetTypeName(this.constructor.name)
    }

    setBaseShape(baseShape: BaseShapeJS) {
        this.baseShape = baseShape
    }

    afterUpdate(force: boolean = false) {
    }

    getTypeName() {
        return this.rawObj.GetTypeName()
    }

    store() {

    }

    getPropertySheet() {
        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)
        if (properties == null)
            return null;

        let componentConfigSheet = {
            key: this.getTypeName(),
            type: PropertyType.COMPONENT,
            config: {
                children: []
            }
        }

        for (let propertyMeta of properties) {
            let propertySheetEntry = propertySheetFactory.createEntry(this, propertyMeta, this.valueChangeHandler)
            if (propertySheetEntry != null) {
                componentConfigSheet.config.children.push(propertySheetEntry)
            }
        }

        let keyFramePropertySheet = propertySheetFactory.createEntryByNameAndCategory("keyframes", PropertyCategory.keyframeArray)

        keyFramePropertySheet["getter"] = this.getKeyFrames.bind(this)
        keyFramePropertySheet["setter"] = this.insertKeyFrames.bind(this) // Same as other arrays, setter is alias of inserter.
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
            if (myPropertySheet)
                propertySheet.addProperty(myPropertySheet)
        }
    }

    cleanUp() {

    }

    setInvisible(){

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
}

export {AbstractComponent, PropertyValue, Component}