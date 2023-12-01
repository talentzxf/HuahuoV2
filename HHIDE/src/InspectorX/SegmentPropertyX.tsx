import * as React from "react"
import {CSSUtils} from "../Utilities/CSSUtils";
import {Vector2PropertyX} from "./Vector2PropertyX";

type SegmentPropertyState = {
    showDetails: boolean
}

class SegmentPropertyX extends React.Component<any, SegmentPropertyState> {
    state: SegmentPropertyState = {
        showDetails: false
    }

    render() {
        let segment = this.props.property.getter()

        return (
            <div className={CSSUtils.getButtonClass("teal") + " rounded hover:cursor-pointer"} onClick={(e) => {
                this.state.showDetails = !this.state.showDetails
                this.setState(this.state)
            }}>
                {"Segment_" + this.props.idx}
                {
                    this.state.showDetails && (
                        <div className="flex flex-col absolute z-10 border-solid bg-gray-500 shadow rounded p-1" style={{
                            left: "50%",
                            top: "50%"
                        }}>
                            <Vector2PropertyX property={{
                                key: "point",
                                getter: () => {
                                    return segment.point
                                }
                            }}></Vector2PropertyX>
                            <Vector2PropertyX property={{
                                key: "handleIn",
                                getter: () => {
                                    return segment.handleIn
                                }
                            }}></Vector2PropertyX>
                            <Vector2PropertyX property={{
                                key: "handleOut",
                                getter: () => {
                                    return segment.handleOut
                                }
                            }}></Vector2PropertyX>
                        </div>
                    )
                }
            </div>
        )
    }
}

export {SegmentPropertyX}