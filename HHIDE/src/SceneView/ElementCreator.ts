import {SceneView} from "./SceneView";
import {findParentPanel} from "../Utilities/PanelUtilities";
import {HHPanel} from "hhpanel";

class ElementCreator{
    sceneView: SceneView
    sceneViewPanel: HHPanel

    onNewElement(){

        this.sceneView = document.querySelector("#mainScene")
        this.sceneViewPanel = findParentPanel(this.sceneView)

        let newEleContent = document.createElement("hh-content")
        newEleContent.title = "NewElement"
        newEleContent.style.width = "100%"
        newEleContent.style.height = "100%"
        newEleContent.style.flexBasis = "100%"
        newEleContent.style.display = "flex"
        newEleContent.style.alignItems = "stretch"

        let elementSceneView = document.createElement("hh-sceneview")
        elementSceneView.style.flexBasis = "100%"
        elementSceneView.style.display = "flex"
        elementSceneView.style.alignItems = "stretch"

        newEleContent.appendChild(elementSceneView)
        this.sceneViewPanel.appendChild(newEleContent)
    }
}

let elementCreator = new ElementCreator()

export {elementCreator}