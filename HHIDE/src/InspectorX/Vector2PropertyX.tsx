import * as React from "react"
import {CSSUtils} from "../Utilities/CSSUtils";
import {Property} from "hhcommoncomponents"
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {eps, PropertyEntry, PropertyProps, registerPropertyChangeListener} from "./BasePropertyX";
import {PropertyChangeListener} from "./PropertyChangeListener";

type Vector2PropertyState = {
    x: number,
    y: number
}

class Vector2PropertyX extends React.Component<PropertyProps, Vector2PropertyState> implements PropertyChangeListener {

    state: Vector2PropertyState = {
        x: -1.0,
        y: -1.0
    }

    valueFieldChanged(val: string, fieldName: string) {
        let property = this.props.property
        let currentValueOrigin = property.getter()

        if (property.setter) {
            let currentValue = {
                x: currentValueOrigin.x,
                y: currentValueOrigin.y
            }
            let newValue = {
                x: currentValueOrigin.x,
                y: currentValueOrigin.y
            }
            newValue[fieldName] = Number(val)

            let command = new SetFieldValueCommand(property.setter, currentValue, newValue)
            undoManager.PushCommand(command)
            command.DoCommand()
        }

        this.state[fieldName] = Number(Number(property.getter()[fieldName].toFixed(2)))
        this.setState(this.state)
    }

    onInputXValueChanged(e) {
        this.valueFieldChanged(e.currentTarget.value, "x")
    }

    onInputYValueChanged(e) {
        this.valueFieldChanged(e.currentTarget.value, "y")
    }

    onValueChanged(val) {
        this.state.x = Number(Number(val.x).toFixed(2))
        this.state.y = Number(Number(val.y).toFixed(2))
        this.setState(this.state)
    }

    render() {
        let property = this.props.property
        let curValue = property.getter()
        if (Math.abs(this.state.x - curValue.x) > eps) {
            this.state.x = Number(Number(curValue.x).toFixed(2))
        }

        if (Math.abs(this.state.y - curValue.y) > eps) {
            this.state.y = Number(Number(curValue.y).toFixed(2))
        }

        registerPropertyChangeListener(this, property)

        return (
            <PropertyEntry property={property}>
                <div className="flex flex-col">
                    <div className="flex flex-row align-middle items-center">
                        <label htmlFor="input_x">X</label>
                        <input id="input_x" className={CSSUtils.getInputStyle()}
                               step={property?.config?.step || 1.0} min={property?.config?.min || null}
                               max={property?.config?.max || null}
                               type={property?.config?.elementType || "number"} value={this.state.x}
                               onChange={this.onInputXValueChanged.bind(this)}>
                        </input>
                    </div>

                    <div className="flex flex-row items-center">
                        <label htmlFor="input_y">Y</label>
                        <input id="input_y" className={CSSUtils.getInputStyle()}
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

export {Vector2PropertyX}