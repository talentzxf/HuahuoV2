enum PropertyType{
    STRING,
    FLOATARRAY,
    FLOAT,
    COLOR
}

class Property{
    key: string
    type: PropertyType
    setter: Function
    getter: Function
}

class PropertySheet{
    protected properties: Array<Property> = new Array<Property>()

    AddProperty(property: Property){
        this.properties.push(property)
    }
}

export {PropertySheet,Property,PropertyType}