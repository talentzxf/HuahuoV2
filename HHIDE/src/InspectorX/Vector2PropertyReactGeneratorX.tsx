import * as React from "react"
import {PropertyEntry} from "./BasePropertyDivGeneratorX";
import {CSSUtils} from "../Utilities/CSSUtils";
import {Property} from "hhcommoncomponents"

type Vector2PropertyProps = {
    property: Property
}

type Vector2PropertyState = {
    x: number,
    y: number
}

class Vector2PropertyReactGeneratorX extends React.Component<Vector2PropertyProps, Vector2PropertyState> {

    state: Vector2PropertyState = {
        x: -1.0,
        y: -1.0
    }

    onInputXValueChanged(e) {

    }

    onInputYValueChanged(e) {

    }

    render() {
        let property = this.props.property
        return (
            <PropertyEntry property={property}>
                <div className="flex flex-col">
                    <div className="flex flex-row">
                        <label>X</label>
                        <input className={CSSUtils.getInputStyle() + " text-right"}
                               step={property?.config?.step || 1.0} min={property?.config?.min || null}
                               max={property?.config?.max || null}
                               type={property?.config?.elementType || "number"} value={this.state.x}
                               onChange={this.onInputXValueChanged.bind(this)}>
                        </input>
                    </div>

                    <div className="flex flex-row">
                        <label>Y</label>
                        <input className={CSSUtils.getInputStyle() + " text-right"}
                               step={property?.config?.step || 1.0} min={property?.config?.min || null}
                               max={property?.config?.max || null}
                               type={property?.config?.elementType || "number"} value={this.state.y}
                               onChange={this.onInputYValueChanged.bind(this)}>
                        </input>
                    </div>
                </div>
            </PropertyEntry>
        );
    }
}

export {Vector2PropertyReactGeneratorX}