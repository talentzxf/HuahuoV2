import {PropertyType} from "hhcommoncomponents";
import * as React from "react"
import {PropertySheet} from "hhcommoncomponents";

class PropertyReactGenerator{
    flexDirection(){
        return "row"
    }

    generateReactElement(property: PropertySheet){
        return (
        <div className={"flex flex-" + this.flexDirection() + " w-full"}>
            {
                property.key && <div>{i18n.t(property.key)}</div>
            }
        </div>
        )
    }
}

let propertyGeneratorMap:Map<PropertyType, PropertyReactGenerator> = new Map<PropertyType, PropertyReactGenerator>()

function RegisterReactGenerator(type: PropertyType, generator: PropertyReactGenerator){
    propertyGeneratorMap.set(type, generator)
}

function GetPropertyReactGenerator(type: PropertyType){
    return propertyGeneratorMap.get(type)
}

export {GetPropertyReactGenerator, RegisterReactGenerator}