import {Property} from "hhcommoncomponents"
import * as React from "react"
import {PropertyEntry} from "./BasePropertyDivGeneratorX";
import {CSSUtils} from "../Utilities/CSSUtils";

type StringPropertyState = {
    value: string
}

class StringPropertyX extends React.Component<any, StringPropertyState> {
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
            return <span className="p-x-1 m-x-1 text-gray-400"> {textValue} </span>
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

    render() {
        return (
            <PropertyEntry property={this.props.property} >
                {
                    this.getContent()
                }
            </PropertyEntry>
        )
    }
}
export {StringPropertyX}