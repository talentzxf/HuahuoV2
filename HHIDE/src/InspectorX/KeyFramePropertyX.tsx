import * as React from "react"
import {PropertyEntry, PropertyProps} from "./BasePropertyX";
import {CSSUtils} from "../Utilities/CSSUtils";

type KeyFramePropertyState = {
    showDetails: boolean
}

class KeyFramePropertyX extends React.Component<PropertyProps, KeyFramePropertyState> {
    state: KeyFramePropertyState = {
        showDetails: false
    }

    render() {
        let property = this.props.property

        let keyFrames = property.getter()

        return <PropertyEntry property={property}>
            <button className={CSSUtils.getButtonClass("primary") + " px-1 rounded"} onClick={() => {
                this.state.showDetails = !this.state.showDetails
                this.setState(this.state)
            }}>{this.state.showDetails ? "^" : "v"}</button>
        </PropertyEntry>
    }
}

export {KeyFramePropertyX}