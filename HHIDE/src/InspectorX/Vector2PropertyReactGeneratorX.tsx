import * as React from "react"
import {PropertyEntry} from "./BasePropertyDivGeneratorX";
import {CSSUtils} from "../Utilities/CSSUtils";
import {Property} from "hhcommoncomponents"
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../RedoUndo/UndoManager";

const eps: number = 0.001

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

    valueFieldChanged(val: string, fieldName: string) {
        let property = this.props.property
        let currentValue = property.getter()

        if (property.setter) {
            let newValue = currentValue.clone()
            newValue[fieldName] = Number(val)

            let command = new SetFieldValueCommand(property.setter, currentValue, newValue)
            undoManager.PushCommand(command)
            command.DoCommand()
        }

        this.state[fieldName] = Number(Number(property.getter()[fieldName].toFixed(2)))
        this.setState(this.state)
    }

    onInputXValueChanged(e) {
        this.valueFieldChanged(e.target.value, "x")
    }

    onInputYValueChanged(e) {
        this.valueFieldChanged(e.target.value, "y")
    }

    onValueChanged(val) {
        this.state.x = Number(Number(val.x).toFixed(2))
        this.state.y = Number(Number(val.y).toFixed(2))
        this.setState(this.state)
    }

    listenerMap: Map<Property, number> = new Map

    render() {
        let property = this.props.property
        let curValue = property.getter()
        if (Math.abs(this.state.x - curValue.x) > eps) {
            this.state.x = Number(Number(curValue.x).toFixed(2))
        }

        if (Math.abs(this.state.y - curValue.y) > eps) {
            this.state.y = Number(Number(curValue.y).toFixed(2))
        }

        if (!this.listenerMap.has(property)) {
            let handlerId = property.registerValueChangeFunc(this.onValueChanged.bind(this))
            this.listenerMap.set(property, handlerId)
        }

        return (
            <PropertyEntry property={property}>
                <div className="flex flex-col">
                    <div className="flex flex-row align-middle items-center">
                        <label htmlFor="input_x">X</label>
                        <input id="input_x" className={CSSUtils.getInputStyle() + " text-right"}
                               step={property?.config?.step || 1.0} min={property?.config?.min || null}
                               max={property?.config?.max || null}
                               type={property?.config?.elementType || "number"} value={this.state.x}
                               onChange={this.onInputXValueChanged.bind(this)}>
                        </input>
                    </div>

                    <div className="flex flex-row items-center">
                        <label htmlFor="input_y">Y</label>
                        <input id="input_y" className={CSSUtils.getInputStyle() + " text-right"}
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