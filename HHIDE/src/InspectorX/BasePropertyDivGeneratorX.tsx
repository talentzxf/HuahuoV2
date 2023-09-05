import * as React from "react"
import {Property, PropertyType} from "hhcommoncomponents";

type PropertyEntryProps = {
    property: Property,
    className?: string
    children,
    noDefaultTitle?: boolean
}

type PropertyEntryState = {}

class PropertyEntry extends React.Component<PropertyEntryProps, PropertyEntryState> {
    getDefaultClassName() {
        return "flex flex-row w-full items-center align-middle"
    }

    render() {
        let property = this.props.property

        return (
            <div className={this.props.className || this.getDefaultClassName()}>
                {
                    !this.props.noDefaultTitle && property.key && <span className="p-1 m-1">{i18n.t(property.key)}</span>
                }
                {this.props.children}
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