import {PropertyType, Property, Logger} from "hhcommoncomponents"

abstract class BasePropertyDesc{
    contentDiv: HTMLDivElement
    titleDiv: HTMLElement
    property: Property
    setter: Function
    handlerId: number

    abstract onValueChanged(val)

    protected constructor(property) {
        this.property = property
        this.setter = this.property.setter

        if(property.registerValueChangeFunc)
            this.handlerId = property.registerValueChangeFunc(this.onValueChanged.bind(this))

        this.contentDiv = document.createElement("div")

        this.titleDiv = document.createElement("div")
        this.titleDiv.innerText = (window as any).i18n.t(property.key)
    }

    clear(){
        if(this.property.unregisterValueChangeFunc)
            this.property.unregisterValueChangeFunc(this.handlerId)
    }

    getTitleDiv(){
        return this.titleDiv
    }

    getContentDiv(){
        return this.contentDiv
    }
}

abstract class BasePropertyDivGenerator {
    abstract generatePropertyDesc(property:Property): BasePropertyDesc

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

function GetPropertyDivGenerator(type: PropertyType){
    return propertyGeneratorMap.get(type)
}

function GenerateDiv(divGenerator: BasePropertyDivGenerator, propertyDesc:BasePropertyDesc){
    let propertyDiv = document.createElement("div")
    propertyDiv.style.flexDirection = divGenerator.flexDirection()
    propertyDiv.style.display = "flex"
    propertyDiv.style.width = "100%"
    propertyDiv.style.gap = "10px"

    if(propertyDesc.getTitleDiv())
        propertyDiv.appendChild(propertyDesc.getTitleDiv())
    propertyDiv.appendChild(propertyDesc.getContentDiv())

    return propertyDiv
}

export {BasePropertyDivGenerator, RegisterDivGenerator, GetPropertyDivGenerator, BasePropertyDesc, GenerateDiv}