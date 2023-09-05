import {Property} from "hhcommoncomponents"
import * as React from "react"
import {PropertyEntry} from "./BasePropertyDivGeneratorX";

type StringPropertyState = {
    value: string
}

class StringPropertyGeneratorX extends React.Component<any, StringPropertyState> {
    constructor(props) {
        super(props);

        this.state.value = props.property.getter()
    }

    state: StringPropertyState = {
        value: "Unknown"
    }

    onTextChanged(e) {
        this.props.property.setter(e.target.value)
        this.state.value = this.props.getter()
        this.setState(this.state)
    }

    getContent(){
        let property = this.props.property

        let textValue = i18n.t(property.getter())

        if (!property.setter) {
            return <span className="text-gray-400"> {textValue} </span>
        } else {
            return (
                <input value={this.state.value} onChange={this.onTextChanged.bind(this)}>
                </input>
            )
        }
    }

    render() {
        let property = this.props.property

        return (
            <PropertyEntry property={property} divStyle="flex flex-row">
                {
                    property.key && <span>{i18n.t(property.key)}</span>
                }
                {
                    this.getContent()
                }
            </PropertyEntry>
        )
    }
}
export {StringPropertyGeneratorX}