import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {
    buildOperator,
    capitalizeFirstLetter,
    PropertyCategory,
    PropertyDef
} from "./PropertySheetBuilder";
import {propertySheetFactory} from "./PropertySheetBuilderFactory"
import {IsValidWrappedObject, PropertyConfig, PropertyType} from "hhcommoncomponents";
import {huahuoEngine} from "../EngineAPI";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {ComponentConfig} from "./ComponentConfig";

const metaDataKey = Symbol("objectProperties")
declare var Module: any;

class FieldShapeArrayIterable{
    fieldShapeArray // Store the cpp side array.
    constructor(fieldShapeArray) {
        this.fieldShapeArray = fieldShapeArray
    }

    [Symbol.iterator](){
        let curIdx = 0
        let _this = this
        const iterator = {
            next(){
                if(curIdx < _this.fieldShapeArray.GetShapeCount()){
                    let targetShape = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(_this.fieldShapeArray.GetShape(curIdx++), true)
                    return {value: targetShape, done: false}
                }

                return {value: null, done: true}
            }
        }

        return iterator
    }
}

function getProperties(target):object[]{
    let properties: object[] = Reflect.getMetadata(metaDataKey, target)
    if (!properties) {
        properties = new Array<PropertyDef>()
        Reflect.defineMetadata(metaDataKey, properties, target)
    }

    return properties
}

function PropertyValue(category:PropertyCategory, initValue = null, config?: PropertyConfig) {
    return function (target: object, propertyKey: string) {
        let properties = getProperties(target)
        let propertyEntry:PropertyDef = {
            key: propertyKey,
            type: category,
            initValue: initValue,
            config: config
        }
        properties.push(propertyEntry)
    }
}

function Component(componentConfig?: ComponentConfig){
    return function(ctor){
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

    private valueChangeHandler:ValueChangeHandler = new ValueChangeHandler()

    callHandlers(propertyName: string, val: any) {
        this.valueChangeHandler.callHandlers(propertyName, val)
    }

    handleInterpolateEntry(propertyEntry){
        let operator = buildOperator(propertyEntry.type, this.rawObj)

        operator.registerField(propertyEntry["key"], propertyEntry["initValue"])

        let fieldName = propertyEntry["key"]
        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName)

        this[setterName] = function(val: number){
            let currentValue = this[fieldName]
            if(operator.isEqual(currentValue, val)){
                return
            }

            this[fieldName] = val
            this.callHandlers(fieldName, val)
            if(this.baseShape){
                this.baseShape.update(true)

                this.baseShape.callHandlers(fieldName, val)
            }
        }

        this[getterName] = function(){
            return operator.getField(fieldName)
        }

        // Remove the property and add setter/getter
        delete this[propertyEntry["key"]]

        // Add getter and setter
        Object.defineProperty(this, propertyEntry["key"], {
            get: function(){
                return this[getterName]()
            },
            set: function(val){
                operator.setField(fieldName, val)
            }
        })

        // Store in cpp side on creation. Or else the information of the first frame might be lost.
        this[fieldName] = this[fieldName]
    }

    handleShapeArrayEntry(propertyEntry){
        let fieldName = propertyEntry["key"]
        this.shapeArrayFieldNames.add(fieldName)
        this.rawObj.RegisterShapeArrayValue(fieldName)

        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName)
        let inserterName = "insert" + capitalizeFirstLetter(fieldName)
        let deleterName = "delete" + capitalizeFirstLetter(fieldName)

        this[getterName] = function(){
            return new FieldShapeArrayIterable(this.rawObj.GetShapeArrayValue(fieldName))
        }.bind(this)

        // This is just alias of the insert funtion.
        this[setterName] = function(val){
            this[inserterName](val)
        }.bind(this)

        this[inserterName] = function (val:BaseShapeJS){
            if(!IsValidWrappedObject(this.rawObj.GetShapeArrayValue(fieldName))){
                this.rawObj.CreateShapeArrayValue(fieldName)
            }

            this.rawObj.GetShapeArrayValueForWrite(fieldName).InsertShape(val.getRawShape())

            if(this.baseShape)
                this.baseShape.update(true)
            this.callHandlers(fieldName, null) // Is the val parameter really matters in this case?
        }.bind(this)

        this[deleterName] = function(val){
            this.rawObj.GetShapeArrayValue(fieldName).DeleteShape(val)
            if(this.baseShape)
                this.baseShape.update(true)

            this.callHandlers(fieldName, null) // Is the val parameter really matters in this case?
        }.bind(this)

        // Add getter and setter
        Object.defineProperty(this, propertyEntry["key"], {
            get: function(){
                return this[getterName]()
            }
        })
    }

    constructor(rawObj?) {
        if(rawObj){
            this.rawObj = castObject( rawObj, Module["CustomComponent"])
        }else{
            this.rawObj = Module["CustomComponent"].prototype.CreateComponent()
        }

        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)

        let _this = this
        properties.forEach(propertyEntry => {
            if(propertyEntry.type == PropertyCategory.interpolateFloat || propertyEntry.type == PropertyCategory.interpolateColor){
                _this.handleInterpolateEntry(propertyEntry)
            } else if(propertyEntry.type == PropertyCategory.shapeArray){
                _this.handleShapeArrayEntry(propertyEntry)
            }
        })

        this.rawObj.SetTypeName(this.constructor.name)
    }

    setBaseShape(baseShape: BaseShapeJS) {
        this.baseShape = baseShape
    }

    afterUpdate(force:boolean = false) {
    }

    getTypeName(){
        return this.rawObj.GetTypeName()
    }

    store(){

    }

    getPropertySheet(){
        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)

        let componentConfigSheet = {
            key: this.getTypeName(),
            type: PropertyType.COMPONENT,
            config: {
                children: []
            }
        }

        for(let propertyMeta of properties){
            let propertySheetEntry = propertySheetFactory.createEntry(this, propertyMeta, this.valueChangeHandler)
            if(propertySheetEntry != null){
                componentConfigSheet.config.children.push(propertySheetEntry)
            }
        }

        return componentConfigSheet
    }

    initPropertySheet(propertySheet){
        if(!this.propertySheetInited){
            this.propertySheetInited = true
            propertySheet.addProperty(this.getPropertySheet())
        }
    }

    cleanUp(){

    }

    // Pass in a set to avoid creation of the set multiple times.
    getReferencedShapes(set: Set<BaseShapeJS>){

        // Put all shapeArray values in the set.
        for( let fieldName of this.shapeArrayFieldNames){
            for(let shape of this[fieldName]){
                set.add(shape)
            }
        }
    }
}

export {AbstractComponent, PropertyValue, Component}