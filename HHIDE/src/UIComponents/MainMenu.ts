import {CustomElement} from "hhcommoncomponents";
import {createRoot} from "react-dom/client";
import {MainMenuX} from "./MainMenuX";
import * as React from "react"

@CustomElement({
    selector: "hh-main-menu"
})
class HHMainMenu extends HTMLElement {
    connectedCallback() {
        let reactRoot = createRoot(this)
        let mainMenuX = React.createElement(MainMenuX)

        reactRoot.render(mainMenuX)
    }
}

export {HHMainMenu}