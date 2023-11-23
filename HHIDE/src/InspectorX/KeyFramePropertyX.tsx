import * as React from "react"
import {PropertyEntry, PropertyProps} from "./BasePropertyX";
import {CSSUtils} from "../Utilities/CSSUtils";

const canvasWidth = 200
const canvasHeight = 50
const penOffset = 10
const textMargin = 5
const spanRectHeight = 5

type KeyFramePropertyState = {
    showDetails: boolean
}

class KeyFramePropertyX extends React.Component<PropertyProps, KeyFramePropertyState> {
    state: KeyFramePropertyState = {
        showDetails: false
    }

    constructor() {
        super();
    }

    render() {
        let property = this.props.property

        let keyFrames = property.getter()

        return <PropertyEntry property={property}>
            <button className={CSSUtils.getButtonClass("primary") + " px-1 rounded"} onClick={() => {
                this.state.showDetails = !this.state.showDetails
                this.setState(this.state)
            }}>{this.state.showDetails ? "^" : "v"}</button>
            {
                this.state.showDetails && <canvas width={canvasWidth} height={canvasHeight} style={{
                    width: canvasWidth + "px",
                    height: canvasHeight + "px"
                }}></canvas>
            }
        </PropertyEntry>
    }
}

export {KeyFramePropertyX}