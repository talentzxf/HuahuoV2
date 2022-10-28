enum PropertyType{
    GROUP,
    PANEL,
    STRING,
    VECTOR2,
    FLOAT,
    COLOR,
    BUTTON,
    REFERENCE,
    ARRAY
}

class Property{
    key: string
    type: PropertyType
    elementType ?: string
    min ?: number // Only number fields need this.
    max ?: number
    step ?: number
    setter?: Function
    getter?: Function
    registerValueChangeFunc?: Function
    unregisterValueChangeFunc?: Function
    action?: Function // For Button property
    children: Array<Property> // For Group property
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