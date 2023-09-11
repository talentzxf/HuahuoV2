import * as React from "react"
import {PropertyProps} from "./BasePropertyX";

class CustomFieldPropertyX extends React.Component<PropertyProps, any> {
    render() {
        let property = this.props.property

        if (property.config && property.config.contentGenerator) {
            let newElement = React.createElement(property.config.contentGenerator, {
                property: property
            })
            return (<div>
                {newElement}
            </div>)
        }

        return null
    }
}

export {CustomFieldPropertyX}