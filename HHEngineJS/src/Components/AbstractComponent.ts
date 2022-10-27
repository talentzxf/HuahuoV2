import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"

const metaDataKey = Symbol("objectProperties")
declare var Module: any;

enum PropertyType{
    interpolate,
    static
}

class PropertyDef{
    key: string
    initValue: object|number
    type: PropertyType
}

function getProperties(target):object[]{
    let properties: object[] = Reflect.getMetadata(metaDataKey, target)
    if (!properties) {
        properties = new Array<PropertyDef>()
        Reflect.defineMetadata(metaDataKey, properties, target)
    }

    return properties
}

function interpolateProperty(initValue: number) {
    return function (target: object, propertyKey: string) {
        let properties = getProperties(target)
        let propertyEntry:PropertyDef = {
            key: propertyKey,
            initValue: initValue,
            type: PropertyType.interpolate
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
            type: PropertyType.static
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

    constructor(rawObj?) {
        if(rawObj){
            this.rawObj = castObject( rawObj, Module["CustomFrameState"])
        }else{
            this.rawObj = Module["CustomFrameState"].prototype.CreateFrameState()
        }

        const properties: string[] = Reflect.getMetadata(metaDataKey, this)

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
}

export {AbstractComponent, interpolateProperty, staticProperty}