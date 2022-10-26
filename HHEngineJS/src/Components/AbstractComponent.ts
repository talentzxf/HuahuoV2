import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import "reflect-metadata"

const metaDataKey = Symbol("interpolateValues")
declare var Module: any;

function interpolateValue(initValue: number) {
    return function (target: object, propertyKey: string) {
        let properties: object[] = Reflect.getMetadata(metaDataKey, target)

        if (properties) {
            let propertyEntry = {
                key: propertyKey,
                initValue: initValue
            }
            properties.push(propertyEntry)
        } else {
            properties = [{key: propertyKey, initValue: initValue}]
            Reflect.defineMetadata(metaDataKey, properties, target)
        }
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
}

export {AbstractComponent, interpolateValue}