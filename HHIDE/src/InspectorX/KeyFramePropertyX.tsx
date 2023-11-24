import * as React from "react"
import {PropertyEntry, PropertyProps} from "./BasePropertyX";
import {CSSUtils} from "../Utilities/CSSUtils";
import {IntArrayX} from "./IntArrayX";

const canvasWidth = 200
const canvasHeight = 50

type KeyFramePropertyState = {
    showDetails: boolean
}

class KeyFramePropertyX extends React.Component<PropertyProps, KeyFramePropertyState> {
    state: KeyFramePropertyState = {
        showDetails: false
    }

    keyFrameEditor() {
        let property = this.props.property

        return <div className="flex flex-col absolute z-10 border-solid bg-gray-500 shadow rounded p-1" style={{
            left: "calc( 50% - " + canvasWidth / 2 + "px",
            top: "50%"
        }}>
            <div className="w-full flex bg-primary-300">
                <span className="w-full">
                    {i18n.t(property.key)}
                </span>
                <button className={CSSUtils.getButtonClass("primary") + " m-0.5 px-1 py-0.5 rounded text-xs"}
                        onClick={(evt) => {
                            this.state.showDetails = false
                            this.setState(this.state)
                            evt.preventDefault()
                        }}>X
                </button>
            </div>
            <IntArrayX width={canvasWidth} height={canvasHeight} property={property}></IntArrayX>
        </div>

    }

    render() {
        let property = this.props.property
        return <PropertyEntry property={property}>
            <button className={CSSUtils.getButtonClass("primary") + " px-1 rounded"} onClick={(evt) => {
                this.state.showDetails = !this.state.showDetails
                this.setState(this.state)
                evt.preventDefault()
            }}>{i18n.t("edit")}</button>
            {
                this.state.showDetails && this.keyFrameEditor()
            }
        </PropertyEntry>
    }
}

export {KeyFramePropertyX}