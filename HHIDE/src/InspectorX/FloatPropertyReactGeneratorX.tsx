import * as React from "react"
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {PropertyEntry} from "./BasePropertyDivGeneratorX";
import {Property} from "hhcommoncomponents"
import {CSSUtils} from "../Utilities/CSSUtils";

const eps: number = 0.01

type FloatPropertyProps = {
    type?: string
    property: Property
}

type FloatPropertyState = {
    value: number
}

class FloatPropertyReactGeneratorX extends React.Component<FloatPropertyProps, FloatPropertyState> {
    state: FloatPropertyState = {
        value: -1.0
    }

    onValueChanged(e) {
        let property = this.props.property
        if (property.setter) {
            let oldValue = Number(property.getter())
            let newValue = Number(e.target.value)
            let command = new SetFieldValueCommand(property.setter, oldValue, newValue)
            undoManager.PushCommand(command)
            command.DoCommand()
        }

        this.state.value = Number(Number(property.getter().toFixed(2)))
        this.setState(this.state)
    }

    render() {
        let property = this.props.property

        if(Math.abs(property.getter() - this.state.value) > eps){
            this.state.value = Number(Number(property.getter().toFixed(2)))
        }

        return (
            <PropertyEntry property={property}>
                <input className={CSSUtils.getInputStyle() + " text-right"}
                       step={property?.config?.step || 1.0} min={property?.config?.min || null}
                       max={property?.config?.max || null}
                       type={this.props.type || "number"} value={this.state.value}
                       onChange={this.onValueChanged.bind(this)}>
                </input>
            </PropertyEntry>
        );
    }
}

export {FloatPropertyReactGeneratorX}