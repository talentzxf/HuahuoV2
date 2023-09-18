import {CustomElement} from "hhcommoncomponents";
import {createRoot} from "react-dom/client";
import {MainMenuX} from "./MainMenuX";
import * as React from "react"
import {huahuoEngine} from "hhenginejs";

@CustomElement({
    selector: "hh-main-menu"
})
class HHMainMenu extends HTMLElement {
    connectedCallback() {
        (window as any).i18n.ExecuteAfterInited(() => {
            let reactRoot = createRoot(this)
            this.style.width = "100%"
            let mainMenuX = React.createElement(MainMenuX)

            reactRoot.render(mainMenuX)
        })
    }
}

export {HHMainMenu}