import {PropertyProps} from "./BasePropertyX";
import * as React from "react"
import {CSSUtils} from "../Utilities/CSSUtils";

type ButtonPropertyState = {
    title: string
}

class ButtonPropertyX extends React.Component<PropertyProps, ButtonPropertyState> {
    state: ButtonPropertyState = {
        title: "Unknown Button"
    }

    syncStateWithProperty() {
        let property = this.props.property

        this.state.title = property.key
        if (property.key instanceof Function) {
            this.state.title = property.key()
        }
    }

    componentDidMount() {
        this.syncStateWithProperty()
    }

    componentDidUpdate(prevProps: Readonly<PropertyProps>, prevState: Readonly<ButtonPropertyState>, snapshot?: any) {
        this.syncStateWithProperty()
    }

    render() {
        let property = this.props.property

        return (<button className={CSSUtils.getButtonClass("indigo") + " m-1"}
                        onClick={(e) => {
                            property.config.action(e)
                            if (property.key instanceof Function) {
                                let candidateKey = property.key()
                                if (candidateKey != this.state.title) {
                                    this.state.title = property.key()
                                    this.setState(this.state)
                                }
                            }
                        }}>{i18n.t(this.state.title)}</button>)
    }
}

export {ButtonPropertyX}