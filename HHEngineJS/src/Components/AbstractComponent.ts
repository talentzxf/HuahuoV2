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

class AbstractComponent {
    rawObj: any = Module["CustomFrameState"].prototype.CreateFrameState()
    baseShape: BaseShapeJS;

    constructor() {
        const properties: string[] = Reflect.getMetadata(metaDataKey, this)

        let _this = this
        properties.forEach(propertyEntry => {
            _this.rawObj.RegisterFloatValue(propertyEntry["key"], propertyEntry["initValue"])
        })
    }

    setBaseShape(baseShape: BaseShapeJS) {
        this.baseShape = baseShape
    }

    afterUpdate() {
    }
}

export {AbstractComponent, interpolateValue}