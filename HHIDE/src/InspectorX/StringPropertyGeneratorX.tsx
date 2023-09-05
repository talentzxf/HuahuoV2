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
        let property = this.props.property

        property.setter(e.target.value)
        this.state.value = property.getter()

        this.setState(this.state)
    }

    getContent(){
        let property = this.props.property

        let textValue = i18n.t(property.getter())

        if (!property.setter) {
            return <span className="p-1 m-1 text-gray-400"> {textValue} </span>
        } else {
            return (
                <input className="p-1 m-1" value={this.state.value} onChange={this.onTextChanged.bind(this)}>
                </input>
            )
        }
    }

    render() {
        let property = this.props.property

        return (
            <PropertyEntry property={property} className="flex flex-row w-full align-middle">
                {
                    property.key && <span className="p-1 m-1">{i18n.t(property.key)}</span>
                }
                {
                    this.getContent()
                }
            </PropertyEntry>
        )
    }
}
export {StringPropertyGeneratorX}