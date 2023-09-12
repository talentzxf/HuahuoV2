import * as React from "react"
import {CSSUtils} from "../Utilities/CSSUtils";
import {PropertyEntry, PropertyProps, registerPropertyChangeListener} from "./BasePropertyX";
import {i18n} from "hhcommoncomponents";
import {PropertyChangeListener} from "./PropertyChangeListener";

type StringPropertyState = {
    value: string
}

class StringPropertyX extends React.Component<PropertyProps, StringPropertyState> implements PropertyChangeListener {
    state: StringPropertyState = {
        value: "Unknown"
    }

    onValueChanged(val: any): void {
        this.state.value = val
        this.setState(this.state)
    }

    onTextChanged(e) {
        let property = this.props.property

        property.setter(e.target.value)
        this.state.value = property.getter()

        this.setState(this.state)
    }

    getContent() {
        let property = this.props.property

        let textValue = i18n.t(property.getter())

        if (!property.setter) {
            return <span className="p-x-1 m-x-1 text-gray-400"> {textValue} </span>
        } else {
            if (property.config && property.config.options && property.config.options.length > 0) {
                return (
                    <select onChange={this.onTextChanged.bind(this)}>
                        {
                            property.config.options.map((option: string, idx: number) => {
                                return (<option key={idx} value={option}>{option}</option>)
                            })
                        }
                    </select>
                )

            } else {
                return (
                    <div>
                        <input className={CSSUtils.getInputStyle()}
                               value={textValue} onChange={this.onTextChanged.bind(this)}>
                        </input>
                    </div>
                )
            }
        }
    }

    render() {
        registerPropertyChangeListener(this, this.props.property)

        return (
            <PropertyEntry property={this.props.property}>
                {
                    this.getContent()
                }
            </PropertyEntry>
        )
    }


}

export {StringPropertyX}