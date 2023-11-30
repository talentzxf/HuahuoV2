import * as React from "react"
import {Vector2PropertyX} from "./Vector2PropertyX";
class SegmentPropertyX extends React.Component<any, any>{
    render() {
        let segment = this.props.property.getter()

        return (
            <div>
                <Vector2PropertyX property={{
                    key:"point",
                    getter: ()=> {
                        return segment.point
                    }
                }}></Vector2PropertyX>
                <Vector2PropertyX property={{
                    key:"handleIn",
                    getter: ()=> {
                        return segment.handleIn
                    }
                }}></Vector2PropertyX>
                <Vector2PropertyX property={{
                    key:"handleOut",
                    getter: ()=> {
                        return segment.handleOut
                    }
                }}></Vector2PropertyX>
            </div>
        );
    }
}

export {SegmentPropertyX}