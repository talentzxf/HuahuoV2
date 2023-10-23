import * as React from "react"
import {Property, PropertyType} from "hhcommoncomponents";
import {PropertyChangeListener} from "./PropertyChangeListener";

const eps = 0.01

interface PropertyEntryProps extends React.HTMLAttributes<HTMLDivElement> {
    property: Property,
    noDefaultTitle?: boolean
}

type PropertyEntryState = {}

class PropertyEntry extends React.Component<PropertyEntryProps, PropertyEntryState> {
    getDefaultClassName() {
        return "flex flex-row w-full items-center align-middle border divide-x"
    }

    render() {
        let {property, noDefaultTitle, children, ...rest} = this.props

        let className = this.props.className || this.getDefaultClassName()
        if (property.singleLine) {
            className += " col-span-2"
        }

        return (
            <div className={className} {...rest}>
                {
                    !noDefaultTitle && property.key &&
                    <span className="px-1 mx-1">{i18n.t(property.key)}</span>
                }
                {children}
            </div>
        )
    }
}

type PropertyProps = {
    property: Property
}

let propertyGeneratorMap: Map<PropertyType, any> = new Map<PropertyType, any>()

function RegisterReactGenerator(type: PropertyType, propertyReactComponent) {
    propertyGeneratorMap.set(type, propertyReactComponent)
}

function GetPropertyReactGenerator(type: PropertyType) {
    return propertyGeneratorMap.get(type)
}

let propertyChangeListenerMap: Map<PropertyChangeListener, Map<Property, number>> = new Map

function registerPropertyChangeListener(listener: PropertyChangeListener, property: Property) {
    if (!property.registerValueChangeFunc) {
        return false
    }

    if (!propertyChangeListenerMap.has(listener)) {
        propertyChangeListenerMap.set(listener, new Map<Property, number>())
    }

    let listenerMap = propertyChangeListenerMap.get(listener)
    if (!listenerMap.has(property)) {
        let listenerId = property.registerValueChangeFunc(listener.onValueChanged.bind(listener))
        listenerMap.set(property, listenerId)

        return true
    }

    return false
}

export {
    GetPropertyReactGenerator,
    RegisterReactGenerator,
    PropertyEntry,
    PropertyProps,
    eps,
    registerPropertyChangeListener
}