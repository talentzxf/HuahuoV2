import * as React from "react"
import {PropertyEntry, PropertyProps} from "./BasePropertyX";
import {CSSUtils} from "../Utilities/CSSUtils";
import {paper} from "hhenginejs"

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

    projectId: number = -1
    bgRectangle
    canvasRef
    constructor(props) {
        super(props);

        this.canvasRef = React.createRef()
    }

    refresh(){

    }

    componentDidMount() {
        if(this.projectId == -1){
            let previousProject = paper.project
            paper.setup(this.canvasRef.current)
            this.projectId = paper.project.index

            this.bgRectangle = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(canvasWidth, canvasHeight))
            this.bgRectangle.fillColor = new paper.Color("lightgray")
            this.refresh()
            previousProject.activate();
        }
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
                    height: canvasHeight + "px",
                    position: "absolute",
                    zIndex: 1,
                    background: "white",
                    border: "1px solid black"
                }}></canvas>
            }
        </PropertyEntry>
    }
}

export {KeyFramePropertyX}