import {CustomElement} from "hhcommoncomponents"
import {createRoot} from "react-dom/client"
import {DrawToolBarX} from "./DrawToolBarX";
import * as React from "react"

@CustomElement({
    selector: "hh-draw-toolbar"
})
class DrawToolBar extends HTMLElement {
    connectedCallback() {
        (window as any).i18n.ExecuteAfterInited(() => {
            // Attach React component
            let reactRoot = createRoot(this)
            let toolBarX = React.createElement(DrawToolBarX)
            reactRoot.render(toolBarX)
        })
    }
}

export {DrawToolBar}