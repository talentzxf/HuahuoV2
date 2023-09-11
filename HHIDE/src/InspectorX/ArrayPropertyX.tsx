import * as React from "react"
import {GetPropertyReactGenerator, PropertyEntry, PropertyProps} from "./BasePropertyX";
import {CSSUtils} from "../Utilities/CSSUtils";
import {ReactNode} from "react";

// TODO: Havn't tested yet.
class ArrayPropertyX extends React.Component<PropertyProps, any> {
    placeHolderElement: ReactNode = null

    onClicked(e: Event) {
        e.preventDefault()
        e.stopPropagation()

        if (this.placeHolderElement == null) {
            this.placeHolderElement = this.createElement()
        }
    }

    createElement() {
        let elementType = this.props.property.elementType
        let generator = GetPropertyReactGenerator(elementType)
        if (generator) {
            return React.createElement(generator, {
                key: Math.random()
            })
        }

        return null
    }

    render() {
        let property = this.props.property

        let index = 0
        let lis = []
        let childElements = property.getter()
        for (let element of childElements) {
            let ele = this.createElement()
            if (ele) {
                childElements.push(ele)
            } else {
                console.warn("Unknown generator:" + element.property.key + " type index:" + element.property.type)
            }
        }

        return (
            <PropertyEntry property={property}>
                <button className={CSSUtils.getButtonClass("indigo")} onClick={this.onClicked.bind(this)}>+</button>
                <div>
                    {
                        this.placeHolderElement
                    }
                    {
                        childElements
                    }
                </div>
            </PropertyEntry>
        )
    }
}

export {ArrayPropertyX}