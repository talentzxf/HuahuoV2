import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"
import {PropertyType} from "hhcommoncomponents";
import {ValueChangeHandler} from "../Shapes/ValueChangeHandler";

const metaDataKey = Symbol("objectProperties")
declare var Module: any;

enum PropertyCategory{
    interpolate,
    static
}

class PropertyDef{
    key: string
    initValue: object|number
    type: PropertyCategory
    minValue?: number = 0.0
    maxValue?: number = 1.0
    step?: number = 0.01
}

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

function capitalizeFirstLetter(str){
    if(str.length == 0)
        return ""

    return str.charAt(0).toUpperCase() + str.slice(1);
}

declare function castObject(obj: any, clz: any): any;

class AbstractComponent {
    rawObj: any;
    baseShape: BaseShapeJS;
    propertySheetInited: boolean = false;

    private valueChangeHandler:ValueChangeHandler = new ValueChangeHandler()

    // Remove these functions later.
    registerValueChangeHandler(valueName: string, preProcessor: Function = null){
        return this.valueChangeHandler.registerValueChangeHandler(valueName, preProcessor)
    }

    unregisterValueChangeHandler(valueName: string) {
        return this.valueChangeHandler.unregisterValueChangeHandler(valueName)
    }

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
                _this.rawObj.SetValue(fieldName, val)
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
                    _this[setterName](val)
                    _this.callHandlers(fieldName, val)
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

                let fieldName = propertyMeta["key"]
                // Generate setter and getter
                let getterName = "get" + capitalizeFirstLetter(fieldName)
                let setterName = "set" + capitalizeFirstLetter(fieldName)

                let propertyDef = {
                    key:"inspector.property" + propertyMeta["key"],
                    elementType: "range",
                    getter: this[getterName].bind(this),
                    setter: this[setterName].bind(this),
                    registerValueChangeFunc: this.registerValueChangeHandler(fieldName),
                    unregisterValueChagneFunc: this.unregisterValueChangeHandler(fieldName)
                }

                if(propertyMeta.type == PropertyCategory.interpolate){
                    propertyDef["type"] = PropertyType.FLOAT
                    propertyDef["min"] = propertyMeta.minValue
                    propertyDef["max"] = propertyMeta.maxValue
                    propertyDef["step"] = propertyMeta.maxValue
                }

                propertySheet.addProperty(propertyDef)
            }
        }
    }
}

export {AbstractComponent, interpolateProperty, staticProperty}