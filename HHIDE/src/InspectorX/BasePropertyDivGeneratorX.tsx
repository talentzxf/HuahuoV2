import * as React from "react"
import {Property, PropertyType} from "hhcommoncomponents";

type PropertyEntryProps = {
    property: Property,
    className?: string
    children
}

type PropertyEntryState = {}

class PropertyEntry extends React.Component<PropertyEntryProps, PropertyEntryState> {
    getDefaultClassName() {
        return "flex flex-row w-full items-center"
    }

    generateTitle(propertyKey: string) {
        return <div className="m-1 p-1 font-bold"> {propertyKey} </div>
    }


    render() {
        return (
            <div className={this.props.className || this.getDefaultClassName()}>
                { this.props.children }
            </div>
        )
    }
}

let propertyGeneratorMap: Map<PropertyType, any> = new Map<PropertyType, any>()

function RegisterReactGenerator(type: PropertyType, propertyReactComponent) {
    propertyGeneratorMap.set(type, propertyReactComponent)
}

function GetPropertyReactGenerator(type: PropertyType) {
    return propertyGeneratorMap.get(type)
}

export {GetPropertyReactGenerator, RegisterReactGenerator, PropertyEntry}