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

        let segmentName = "Segment_" + this.props.idx

        function storeProperty(propertyName, val){
            segment[propertyName].x = val.x
            segment[propertyName].y = val.y

            let shape = segment.path.data.meta
            shape.store({position: true, segments: true})
        }

        let property = this.props.property

        return (
            <div className={CSSUtils.getButtonClass("teal") + " rounded hover:cursor-pointer"} onClick={(e) => {
                this.state.showDetails = true
                this.setState(this.state)
                e.stopPropagation()
                e.preventDefault()
            }}>
                {segmentName}
                {
                    this.state.showDetails && (
                        <div className="flex flex-col absolute z-10 border-solid bg-gray-500 shadow rounded p-1" style={{
                            left: "50%",
                            top: "50%"
                        }}>
                            <div className="w-full flex bg-primary-300">
                                <span className="w-full">
                                    {i18n.t(segmentName)}
                                </span>
                                <button
                                    className={CSSUtils.getButtonClass("primary") + " m-0.5 px-1 rounded text-xs"}
                                    onClick={(evt) => {
                                        this.state.showDetails = false
                                        this.setState(this.state)
                                        evt.preventDefault()
                                        evt.stopPropagation()
                                    }}>X
                                </button>
                            </div>
                            <Vector2PropertyX property={{
                                key: "point",
                                getter: () => {
                                    return segment.point
                                },
                                setter: (val)=> {
                                    storeProperty("point", val)
                                }
                            }}></Vector2PropertyX>
                            <Vector2PropertyX property={{
                                key: "handleIn",
                                getter: () => {
                                    return segment.handleIn
                                },
                                setter: (val)=> {
                                    storeProperty("handleIn", val)
                                }
                            }}></Vector2PropertyX>
                            <Vector2PropertyX property={{
                                key: "handleOut",
                                getter: () => {
                                    return segment.handleOut
                                },
                                setter: (val)=> {
                                    storeProperty("handleOut", val)
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