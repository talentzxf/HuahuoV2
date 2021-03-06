enum PropertyType{
    STRING,
    VECTOR2,
    FLOAT,
    COLOR,
    BUTTON
}

class Property{
    key: string
    type: PropertyType
    setter?: Function
    getter?: Function
    registerValueChangeFunc?: Function
    unregisterValueChangeFunc?: Function

    action?: Function
}

class PropertySheet{
    protected properties: Array<Property> = new Array<Property>()

    addProperty(property: Property){
        this.properties.push(property)
    }

    getProperties(){
        return this.properties
    }

    getProperty(key: string){
        for(let property of this.properties){
            if(property.key == key)
                return property
        }
        return null
    }
}

export {PropertySheet,Property,PropertyType}