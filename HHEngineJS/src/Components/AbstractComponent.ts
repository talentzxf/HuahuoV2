import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";
import {PropertyCategory, PropertyDef, capitalizeFirstLetter} from "./PropertySheetBuilder";
import {propertySheetFactory} from "./PropertySheetBuilderFactory"

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

function interpolateProperty(initValue: number, minValue: number, maxValue: number, step: number) {
    return function (target: object, propertyKey: string) {
        let properties = getProperties(target)
        let propertyEntry:PropertyDef = {
            key: propertyKey,
            initValue: initValue,
            type: PropertyCategory.interpolate,
            minValue: minValue,
            maxValue: maxValue,
            step: step
        }
        properties.push(propertyEntry)
    }
}

function staticProperty(initValue?: object){
    return function (target: object, propertyKey: string) {
        let properties = getProperties(target)
        let propertyEntry:PropertyDef = {
            key: propertyKey,
            initValue: initValue,
            type: PropertyCategory.static
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

    constructor(rawObj?) {
        if(rawObj){
            this.rawObj = castObject( rawObj, Module["CustomFrameState"])
        }else{
            this.rawObj = Module["CustomFrameState"].prototype.CreateFrameState()
        }

        const properties: PropertyDef[] = Reflect.getMetadata(metaDataKey, this)

        let _this = this
        properties.forEach(propertyEntry => {
            _this.rawObj.RegisterFloatValue(propertyEntry["key"], propertyEntry["initValue"])

            let fieldName = propertyEntry["key"]
            // Generate setter and getter
            let getterName = "get" + capitalizeFirstLetter(fieldName)
            let setterName = "set" + capitalizeFirstLetter(fieldName)

            _this[setterName] = function(val: number){
                _this[fieldName] = val
                _this.callHandlers(fieldName, val)
                if(_this.baseShape)
                    _this.baseShape.update()
            }

            _this[getterName] = function(){
                return _this.rawObj.GetValue(fieldName)
            }

            // Remove the property and add setter/getter
            delete _this[propertyEntry["key"]]

            // Add getter and setter
            Object.defineProperty(_this, propertyEntry["key"], {
                get: function(){
                    return _this[getterName]()
                },
                set: function(val){
                    _this.rawObj.SetValue(fieldName, val)
                }
            })
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

export {AbstractComponent, interpolateProperty, staticProperty}