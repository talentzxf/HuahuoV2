import {BaseShapeJS} from "../Shapes/BaseShapeJS";

declare var Module: any;

function interpolateValue(){
    return function(target: AbstractComponent, propertyKey: string){
        target.registerField(propertyKey)
    }
}

class AbstractComponent {
    rawObj: any = new Module["CustomFrameState"]()
    baseShape: BaseShapeJS;

    constructor() {
    }

    registerField(fieldName: string){
        this.rawObj.RegisterFloatValue( fieldName, this.rawObj[fieldName] )
    }

    setBaseShape(baseShape: BaseShapeJS){
        this.baseShape = baseShape
    }

    afterUpdate(){}
}

export {AbstractComponent, interpolateValue}