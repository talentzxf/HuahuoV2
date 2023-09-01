import * as React from "react"
import {HHReactForm} from "../Utilities/HHForm";

class LoginFormX implements HHReactForm {
    render() {
        return (
            <div className="static block bg-zinc-700">
                Hello from React
            </div>
        )
    }

    closeForm() {
    }
}

export {LoginFormX}