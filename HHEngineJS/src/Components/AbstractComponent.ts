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

const metaDataKey = Symbol("objectProperties")
declare var Module: any;

const eps:number = 0.001;
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

declare function castObject(obj: any, clz: any): any;

class AbstractComponent {
    rawObj: any;
    baseShape: BaseShapeJS;
    propertySheetInited: boolean = false;

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
            if(Math.abs(currentValue - val) < eps){
                return
            }

            this[fieldName] = val
            this.callHandlers(fieldName, val)
            if(this.baseShape){
                this.baseShape.update()

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
    }

    handleShapeArrayEntry(propertyEntry){
        this.rawObj.RegisterShapeArrayValue(propertyEntry["key"])

        let fieldName = propertyEntry["key"]
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
                this.baseShape.update()
            this.callHandlers(fieldName, null) // Is the val parameter really matters in this case?
        }.bind(this)

        this[deleterName] = function(val){
            this.rawObj.GetShapeArrayValue(fieldName).DeleteShape(val)
            if(this.baseShape)
                this.baseShape.update()

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
            this.rawObj = castObject( rawObj, Module["CustomFrameState"])
        }else{
            this.rawObj = Module["CustomFrameState"].prototype.CreateFrameState()
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
    }

    setBaseShape(baseShape: BaseShapeJS) {
        this.baseShape = baseShape
    }

    afterUpdate() {
    }

    getTypeName(){
        return this.rawObj.GetTypeName()
    }

    store(){

    }

    initPropertySheet(propertySheet){
        if(!this.propertySheetInited){
            this.propertySheetInited = true

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

            propertySheet.addProperty(componentConfigSheet)
        }
    }

    cleanUp(){

    }
}

export {AbstractComponent, PropertyValue}