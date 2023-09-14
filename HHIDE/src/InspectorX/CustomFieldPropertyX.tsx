import * as React from "react"
import {PropertyEntry, PropertyProps} from "./BasePropertyX";

class CustomFieldPropertyX extends React.Component<PropertyProps, any> {
    render() {
        let property = this.props.property

        if (property.config && property.config.contentGenerator) {
            let newElement = React.createElement(property.config.contentGenerator, {
                property: property
            })
            return (<PropertyEntry property={this.props.property}>
                {newElement}
            </PropertyEntry>)
        }

        return null
    }
}

export {CustomFieldPropertyX}