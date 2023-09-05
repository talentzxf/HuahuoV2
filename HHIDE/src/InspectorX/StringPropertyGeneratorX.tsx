import {Property} from "hhcommoncomponents"
import * as React from "react"
import {PropertyEntry} from "./BasePropertyDivGeneratorX";
import {CSSUtils} from "../Utilities/CSSUtils";

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
                <input className={CSSUtils.getInputStyle()}
                       value={textValue} onChange={this.onTextChanged.bind(this)}>
                </input>
            )
        }
    }

    render() {
        return (
            <PropertyEntry property={this.props.property} className="flex flex-row w-full align-middle">
                {
                    this.getContent()
                }
            </PropertyEntry>
        )
    }
}
export {StringPropertyGeneratorX}