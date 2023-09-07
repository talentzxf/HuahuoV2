import * as React from "react";
import {GetPropertyReactGenerator, PropertyEntry, PropertyProps} from "./BasePropertyX";

type ComponentPropertyState = {
    isOpen: boolean
}

class ComponentPropertyX extends React.Component<PropertyProps, ComponentPropertyState> {
    state: ComponentPropertyState = {
        isOpen: true
    }

    onClick(e) {
        this.state.isOpen = !this.state.isOpen
        this.setState(this.state)
    }

    render() {
        let property = this.props.property

        if (!property.config || !property.config.children || property.config.children.length == 0) {
            return null
        }

        let index = 0

        let childElements = []

        for (let childProperty of property.config.children) {
            if (childProperty.hide)
                continue
            // In child property, setter is actually inserter if not explicitly set.
            if (childProperty.inserter && !childProperty.hasOwnProperty("setter")) {
                childProperty.setter = childProperty.inserter
            }

            let generator = GetPropertyReactGenerator(childProperty.type)
            if (generator) {
                let childReactElement = React.createElement(generator, {
                    key: index++,
                    property: childProperty
                })
                childElements.push(childReactElement)
            }
        }

        let propertyKey = i18n.t(this.props.property.key)

        return (
            <PropertyEntry
                className="flex flex-col w-full first:rounded-t-lg last:rounded-b-lg border border-neutral-200 bg-white"
                property={this.props.property}
                noDefaultTitle={true}>

                <button className="hover:bg-blue-300 bg-blue-100 w-full focus:ring-4 focus:ring-cyan-300 focus:border-cyan-200 text-left
            focus:outline-none" onClick={this.onClick.bind(this)}>
                    {propertyKey}
                </button>

                {
                    this.state.isOpen && (<div className="grid grid-cols-2 animate-[fade-in_1s_ease-in-out]">
                        {childElements}
                    </div>)
                }

            </PropertyEntry>
        )
    }
}

export {ComponentPropertyX}