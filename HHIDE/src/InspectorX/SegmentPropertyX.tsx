import * as React from "react"
import {CSSUtils} from "../Utilities/CSSUtils";
import {Vector2PropertyX} from "./Vector2PropertyX";

type SegmentEditorProps = {
    segmentName,
    segment,
    onClose: Function
}

type SegmentEditorState = {
    left,
    top
}

class SegmentEditor extends React.Component<SegmentEditorProps, SegmentEditorState> {
    state: SegmentEditorState = {
        left: "50%",
        top: "50%"
    }

    rootDivRef
    constructor(props) {
        super(props);
        this.rootDivRef = React.createRef()
    }

    componentDidMount() {
        setTimeout(()=>{
            if(this.rootDivRef.current != null){
                let half_width = this.rootDivRef.current.offsetWidth/2
                let half_height = this.rootDivRef.current.offsetHeight/2

                this.state.left = "calc(50% - " + half_width + "px)"
                this.state.top = "calc(50% - " + half_height + "px)"

                this.props.segment.selected = true

                this.setState(this.state)
            }
        })
    }

    render() {
        let segment = this.props.segment

        function storeProperty(propertyName, val) {
            segment[propertyName].x = val.x
            segment[propertyName].y = val.y

            let shape = segment.path.data.meta
            shape.store({position: true, segments: true})
        }

        return (<div ref={this.rootDivRef} className="flex flex-col absolute z-10 border-solid bg-gray-500 shadow rounded p-1" style={{
            left: this.state.left,
            top: this.state.top
        }}>
            <div className="w-full flex bg-primary-300">
                                <span className="w-full">
                                    {i18n.t(this.props.segmentName)}
                                </span>
                <button
                    className={CSSUtils.getButtonClass("primary") + " m-0.5 px-1 rounded text-xs"}
                    onClick={(evt) => {
                        this.props.onClose()
                        this.props.segment.selected = false
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
                setter: (val) => {
                    storeProperty("point", val)
                }
            }}></Vector2PropertyX>
            <Vector2PropertyX property={{
                key: "handleIn",
                getter: () => {
                    return segment.handleIn
                },
                setter: (val) => {
                    storeProperty("handleIn", val)
                }
            }}></Vector2PropertyX>
            <Vector2PropertyX property={{
                key: "handleOut",
                getter: () => {
                    return segment.handleOut
                },
                setter: (val) => {
                    storeProperty("handleOut", val)
                }
            }}></Vector2PropertyX>
        </div>)
    }
}

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

        return (
            <div className={CSSUtils.getButtonClass("teal") + " rounded hover:cursor-pointer"} onClick={(e) => {
                this.state.showDetails = true
                this.setState(this.state)
                e.stopPropagation()
                e.preventDefault()
            }}>
                {segmentName}
                {
                    this.state.showDetails &&
                    <SegmentEditor segmentName={segmentName} segment={segment} onClose={() => {
                        this.state.showDetails = false
                        this.setState(this.state)
                    }}></SegmentEditor>
                }
            </div>
        )
    }
}

export {SegmentPropertyX}