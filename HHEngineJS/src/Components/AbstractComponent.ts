import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
import {propertySheetFactory} from "./PropertySheetBuilderFactory"
import {PropertyConfig, PropertyType} from "hhcommoncomponents";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {ComponentConfig} from "./ComponentConfig";
import {interpolateVariableHandler} from "./VariableHandlers/InterpolateVariableHandler";
import {shapeArrayHandler} from "./VariableHandlers/ShapeArrayHandler";
import {colorStopArray} from "./VariableHandlers/ColorArrayHandler";

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

        let _this = this
        properties.forEach(propertyEntry => {
            if (propertyEntry.type == PropertyCategory.interpolateFloat || propertyEntry.type == PropertyCategory.interpolateColor) {
                interpolateVariableHandler.handleInterpolateEntry(this, propertyEntry)
            } else if (propertyEntry.type == PropertyCategory.shapeArray) {
                shapeArrayHandler.handleShapeArrayEntry(this, propertyEntry)
            } else if (propertyEntry.type == PropertyCategory.colorStopArray) {
                colorStopArray.handleColorStopArrayEntry(this, propertyEntry)
            }
        })

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

        return componentConfigSheet
    }

    initPropertySheet(propertySheet) {
        if (!this.propertySheetInited) {
            this.propertySheetInited = true
            propertySheet.addProperty(this.getPropertySheet())
        }
    }

    cleanUp() {

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