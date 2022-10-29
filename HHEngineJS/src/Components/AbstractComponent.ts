import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {capitalizeFirstLetter, PropertyCategory, PropertyDef} from "./PropertySheetBuilder";
import {propertySheetFactory} from "./PropertySheetBuilderFactory"
import {PropertyConfig} from "hhcommoncomponents";

const metaDataKey = Symbol("objectProperties")
declare var Module: any;

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

    handleFloatEntry(propertyEntry){
        this.rawObj.RegisterFloatValue(propertyEntry["key"], propertyEntry["initValue"])

        let fieldName = propertyEntry["key"]
        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setterName = "set" + capitalizeFirstLetter(fieldName)

        this[setterName] = function(val: number){
            this[fieldName] = val
            this.callHandlers(fieldName, val)
            if(this.baseShape)
                this.baseShape.update()
        }

        this[getterName] = function(){
            return this.rawObj.GetValue(fieldName)
        }

        // Remove the property and add setter/getter
        delete this[propertyEntry["key"]]

        // Add getter and setter
        Object.defineProperty(this, propertyEntry["key"], {
            get: function(){
                return this[getterName]()
            },
            set: function(val){
                this.rawObj.SetValue(fieldName, val)
            }
        })
    }

    handleShapeArrayEntry(propertyEntry){
        let fieldName = propertyEntry["key"]
        // Generate setter and getter
        let getterName = "get" + capitalizeFirstLetter(fieldName)
        let setName = "set" + capitalizeFirstLetter(fieldName)
        let inserterName = "insert" + capitalizeFirstLetter(fieldName)
        let deleterName = "delete" + capitalizeFirstLetter(fieldName)

        let internalFieldName = "_internal" + capitalizeFirstLetter(fieldName) // This is temporary, we should save it into Cpp side later.
        this[internalFieldName] = new Array()
        this[getterName] = function(){
            return internalFieldName
        }.bind(this)

        // This is just alias of the insert funtion.
        this[setName] = function(val){
            this[inserterName](val)
        }.bind(this)

        this[inserterName] = function (val){
            if(this[internalFieldName] == null){
                this[internalFieldName] = []
            }

            this[internalFieldName].push(val)
            this.callHandlers(fieldName, this[internalFieldName])
        }.bind(this)

        this[deleterName] = function(val){
            if(this[internalFieldName] == null)
                return
            this[internalFieldName] = this[internalFieldName].filter((v)=>{
                return v!= val
            })

            this.callHandlers(fieldName, this[internalFieldName])
        }.bind(this)
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
            if(propertyEntry.type == PropertyCategory.interpolate){
                _this.handleFloatEntry(propertyEntry)
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

            for(let propertyMeta of properties){
                let propertySheetEntry = propertySheetFactory.createEntry(this, propertyMeta, this.valueChangeHandler)
                if(propertySheetEntry != null)
                    propertySheet.addProperty(propertySheetEntry)
            }
        }
    }
}

export {AbstractComponent, PropertyValue}