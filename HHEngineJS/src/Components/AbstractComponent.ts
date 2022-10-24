declare var Module: any;

function interpolateValue(){
    return function(target: AbstractComponent, propertyKey: string){
        target.registerField(propertyKey)
    }
}

class AbstractComponent {
    rawObj: any = new Module["CustomFrameState"]()

    fieldNameIdMap: Map<string, number> = new Map()
    fieldIdNameMap: Map<number, string> = new Map()

    constructor() {
    }

    registerField(fieldName: string){
        this.rawObj.RegisterFloatValue( fieldName, this.rawObj[fieldName] )
    }
}

export {AbstractComponent, interpolateValue}