import * as React from "react"
import {GetPropertyReactGenerator, PropertyEntry, PropertyProps} from "./BasePropertyX";
import {CSSUtils} from "../Utilities/CSSUtils";

class ArrayPropertyX extends React.Component<PropertyProps, any> {
    onClicked(e: Event) {
        e.preventDefault()
        e.stopPropagation()

        let property = this.props.property
        property.inserter(property.initValue)

        this.forceUpdate()
    }

    createReactNode(childElement, idx) {
        let property = this.props.property
        let elementType = property.elementType
        let generator = GetPropertyReactGenerator(elementType)
        if (generator) {
            return React.createElement(generator, {
                key: idx,
                property: {
                    setter: (shape) => {
                        property.updater(idx, shape)
                    },
                    getter: () => {
                        return childElement
                    }
                }
            })
        }

        return null
    }

    render() {
        let property = this.props.property

        let idx = 0
        let reactElements = []
        let childElements = property.getter()
        for (let element of childElements) {
            let ele = this.createReactNode(element, idx++)
            if (ele) {
                reactElements.push(ele)
            } else {
                console.warn("Unknown generator:" + element.property.key + " type index:" + element.property.type)
            }
        }

        return (
            <PropertyEntry property={property}>
                <button className={CSSUtils.getButtonClass("indigo")} onClick={this.onClicked.bind(this)}>+</button>
                <div>
                    {
                        reactElements
                    }
                </div>
            </PropertyEntry>
        )
    }
}

export {ArrayPropertyX}