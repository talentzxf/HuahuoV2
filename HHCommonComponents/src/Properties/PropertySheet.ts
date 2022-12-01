import {PropertyConfig} from "./PropertyConfig";

enum PropertyType{
    GROUP,
    PANEL,
    BOOLEAN,
    STRING,
    VECTOR2,
    VECTOR3,
    FLOAT,
    COLOR,
    BUTTON,
    REFERENCE,
    ARRAY,
    COMPONENT,
    COLORSTOPARRAY,
    KEYFRAMES
}

class Property{
    key: string
    type: PropertyType
    component: any
    setter?: Function // Setter will be used as inserter if the property is an array.
    getter?: Function
    updater?: Function // updater and deleter are used if the property is an array.
    deleter?: Function
    registerValueChangeFunc?: Function
    unregisterValueChangeFunc?: Function
    config: PropertyConfig
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