import * as React from "react"
import {SVGFiles} from "../Utilities/Svgs";

class ToolBarX extends React.Component<any, any> {
    render() {
        return (
            <div>
                <button className="w-[40px] h-[40px]"><img
                    src={`data:image/svg+xml;utf8,${encodeURIComponent(SVGFiles.saveBtn)}`}/></button>
            </div>
        )
    }
}

export {ToolBarX}