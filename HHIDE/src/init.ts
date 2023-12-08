import {huahuoEngine, renderEngine2D} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {shortcutsManager} from "./Shortcuts/ShortcutsManager";
import {EditorShapeProxy} from "./ShapeDrawers/EditorShapeProxy";
import {createRoot} from "react-dom/client"
import {findParentSideBar} from "hhpanel"
import * as React from "react"
import {InspectorX} from "./InspectorX/InspectorX";
import {HierarchyX} from "./Hierarchy/HierarchyX";
import {FileExplorerX} from "./FileExplorer/FileExplorerX";

function showMainDiv() {
    let loadingElement = document.querySelector("#loading") as HTMLDivElement
    loadingElement.style.display = "none"
    document.body.style.removeProperty("overflow")
    let mainDiv = document.querySelector("#mainSceneDiv") as HTMLDivElement
    mainDiv.style.display = "block"
}

let i18n = (window as any).i18n

function updateMousePos(evt: MouseEvent) {
    let posX = evt.offsetX
    let posY = evt.offsetY

    let globalPosition = renderEngine2D.getGlobalPosition(posX, posY)

    let statusBarSpan = document.querySelector("#statusBarSpan")
    statusBarSpan.innerHTML = i18n.t("statusbar.mousePosition", {
        mouseX: Math.round(globalPosition.x),
        mouseY: Math.round(globalPosition.y)
    })
}

function setPrompt(promptMsg: string) {
    let promptBarSpan = document.querySelector("#promptBarSpan")

    promptBarSpan.innerHTML = promptMsg
}

function clearPrompt() {
    setPrompt("  ")
}

function createReactComponent(divName, reactComponent) {
    let div = document.getElementById(divName)
    let root = createRoot(div)
    let element = React.createElement(reactComponent)
    root.render(element)
}

function init() {
    huahuoEngine.ExecuteAfterInited(() => {
        i18n.ExecuteAfterInited(() => {
            HHToast.info(i18n.t("welcomeMsg"))
            showMainDiv()

            document.body.addEventListener("mousemove", updateMousePos)

            shortcutsManager.init()

            huahuoEngine.setShapeDecorator((shape) => {
                return EditorShapeProxy.CreateProxy(shape, false)
            })

            // Attach inspector from react
            createReactComponent("hh-inspector", InspectorX)

            // Attach hierarchy from react
            createReactComponent("hh-hierarchy", HierarchyX)

            createReactComponent("hh-file-explorer", FileExplorerX)
        })
    })
}

export {init, setPrompt, clearPrompt}