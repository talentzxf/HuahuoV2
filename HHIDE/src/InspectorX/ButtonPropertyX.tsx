import {PropertyProps} from "./BasePropertyX";
import * as React from "react"
import {CSSUtils} from "../Utilities/CSSUtils";

class ButtonPropertyX extends React.Component<PropertyProps, any> {
    render() {
        let property = this.props.property
        return (<button className={CSSUtils.getButtonClass("indigo") + " m-1"}
                        onClick={property.config.action}>{i18n.t(property.key)}</button>)
    }
}

export {ButtonPropertyX}