enum PropertyType{
    STRING,
    VECTOR2,
    FLOAT,
    COLOR
}

class Property{
    key: string
    type: PropertyType
    setter: Function
    getter: Function
    registerValueChangeFunc: Function
    unregisterValueChangeFunc: Function
}

class PropertySheet{
    protected properties: Array<Property> = new Array<Property>()

    addProperty(property: Property){
        this.properties.push(property)
    }

    getProperties(){
        return this.properties
    }
}

export {PropertySheet,Property,PropertyType}