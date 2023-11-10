import * as React from "react"
import {GetPropertyReactGenerator, PropertyEntry, PropertyProps, registerPropertyChangeListener} from "./BasePropertyX";
import {CSSUtils} from "../Utilities/CSSUtils";
import {PropertyChangeListener} from "./PropertyChangeListener";

class ArrayPropertyX extends React.Component<PropertyProps, any> implements PropertyChangeListener {
    onClicked(e: Event) {
        e.preventDefault()
        e.stopPropagation()

        let property = this.props.property
        property.inserter(property.initValue)

        this.forceUpdate()
    }

    createReactNode(idx) {
        let property = this.props.property
        let elementType = property.elementType
        let generator = GetPropertyReactGenerator(elementType)
        if (generator) {
            let props = {
                key: idx,
                property: {
                    getter: () => {
                        return property.getter()[idx]
                    }
                }
            }

            if (property.updater != null && property.updater instanceof Function) {
                props.property["setter"] = (val) => {
                    return property.updater(idx, val)
                }
            }

            return React.createElement(generator, props)
        }

        return null
    }

    render() {
        let property = this.props.property

        registerPropertyChangeListener(this, property)

        let idx = 0
        let reactElements = []
        let childElements = property.getter()
        for (let element of childElements) {
            let label = null
            if (property.getLabel && property.getLabel instanceof Function) {
                label = <span>{i18n.t(property.getLabel(idx))}</span>
            }

            let ele = this.createReactNode(idx)
            if (ele) {
                if (label != null) {
                    reactElements.push(<div style={{display: "flex", alignItems: "center"}}
                                            key={idx}>{label} {ele} </div>)
                } else {
                    reactElements.push(ele)
                }
            } else {
                // console.warn("Unknown generator:" + element.property.key + " type index:" + element.property.type)
            }
            idx++
        }

        return (
            <PropertyEntry className="col-span-2" property={property}>
                {
                    property.inserter != null && property.inserter instanceof Function &&
                    <button className={CSSUtils.getButtonClass("indigo")} onClick={this.onClicked.bind(this)}>+</button>
                }

                <div className="grid grid-cols-2 gap-2">
                    {
                        reactElements
                    }
                </div>
            </PropertyEntry>
        )
    }

    onValueChanged(val: any): void {
        this.forceUpdate()
    }
}

export {ArrayPropertyX}