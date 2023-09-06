import * as React from "react"
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {Property} from "hhcommoncomponents"
import {CSSUtils} from "../Utilities/CSSUtils";
import {PropertyEntry} from "./BasePropertyX";

const eps: number = 0.01

type FloatPropertyProps = {
    property: Property
}

type FloatPropertyState = {
    value: number
}

class FloatPropertyX extends React.Component<FloatPropertyProps, FloatPropertyState> {
    state: FloatPropertyState = {
        value: -1.0
    }

    listenerMap: Map<Property, number> = new Map

    onInputValueChanged(e) {
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

    onValueChanged(val){
        this.state.value = Number(Number(val.toFixed(2)))
        this.setState(this.state)
    }

    render() {
        let property = this.props.property

        if(Math.abs(property.getter() - this.state.value) > eps){
            this.state.value = Number(Number(property.getter().toFixed(2)))
        }

        if(!this.listenerMap.has(property)){
            let handlerId = property.registerValueChangeFunc(this.onValueChanged.bind(this))
            this.listenerMap.set(property, handlerId)
        }

        return (
            <PropertyEntry property={property}>
                <div>
                    <input className={CSSUtils.getInputStyle()}
                           step={property?.config?.step || 1.0} min={property?.config?.min || null}
                           max={property?.config?.max || null}
                           type={property?.config?.elementType || "number"} value={this.state.value}
                           onChange={this.onInputValueChanged.bind(this)}>
                    </input>
                </div>
            </PropertyEntry>
        );
    }
}

export {FloatPropertyX}