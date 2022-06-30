import {SceneView} from "./SceneView";
import {findParentPanel} from "../Utilities/PanelUtilities";
import {HHPanel} from "hhpanel";
import {renderEngine2D, huahuoEngine} from "hhenginejs"
import {HHContent, PanelEventNames} from "hhpanel";

class ElementCreator{
    sceneView: SceneView
    sceneViewPanel: HHPanel

    constructor() {
        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            _this.sceneView = document.querySelector("#mainScene")
            _this.sceneViewPanel = findParentPanel(this.sceneView)

            _this.sceneViewPanel.addEventListener(PanelEventNames.CONTENTSELECTED, _this.onContentSelected.bind(_this))
        })
    }

    onContentSelected(e){
        let content: HHContent = e.detail.content
        let canvas = content.querySelector("canvas")
        renderEngine2D.setDefaultCanvas(canvas)
    }

    onNewElement(){
        let newEleContent = document.createElement("hh-content")
        newEleContent.title = "NewElement"
        newEleContent.style.width = "100%"
        newEleContent.style.height = "100%"
        newEleContent.style.flexBasis = "100%"
        newEleContent.style.alignItems = "stretch"

        let elementSceneView = document.createElement("hh-sceneview")
        elementSceneView.id = "NewElement"
        elementSceneView.style.flexBasis = "100%"
        elementSceneView.style.display = "flex"
        elementSceneView.style.alignItems = "stretch"

        newEleContent.appendChild(elementSceneView)
        let idx = this.sceneViewPanel.addContent(newEleContent)
        this.sceneViewPanel.selectTab(idx)
    }
}

let elementCreator = new ElementCreator()

export {elementCreator}