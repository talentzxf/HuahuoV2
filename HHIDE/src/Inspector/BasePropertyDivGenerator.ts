import {PropertyType, Property, Logger} from "hhcommoncomponents"

abstract class BasePropertyDivGenerator {
    abstract generateDiv(property:Property): HTMLElement

    flexDirection(){
        return "row"
    }
}

let propertyGeneratorMap:Map<PropertyType, BasePropertyDivGenerator> = new Map<PropertyType, BasePropertyDivGenerator>()

function RegisterDivGenerator(type: PropertyType, generator: BasePropertyDivGenerator): boolean{
    if(propertyGeneratorMap.has(type)){
        Logger.error("The generator is already registered!")
        return false;
    }

    propertyGeneratorMap.set(type, generator);
}

function GeneratePropertyDiv(type: PropertyType, property: Property): HTMLElement{
    return propertyGeneratorMap.get(type).generateDiv(property)
}

function GetPropertyFlexDirection(type: PropertyType){
    return propertyGeneratorMap.get(type).flexDirection()
}

export {BasePropertyDivGenerator, RegisterDivGenerator, GeneratePropertyDiv, GetPropertyFlexDirection}