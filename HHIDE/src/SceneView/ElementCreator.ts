import {SceneView} from "./SceneView";
import {findParentPanel} from "../Utilities/PanelUtilities";
import {HHPanel} from "hhpanel";
import {renderEngine2D, huahuoEngine, ElementShapeJS, paper} from "hhenginejs"
import {HHContent, PanelEventNames} from "hhpanel";
import {HHTimeline} from "hhtimeline"
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";

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

        let sceneview = content.querySelector("hh-sceneview")

        this.sceneView = sceneview
        this.sceneViewPanel = findParentPanel(this.sceneView)
        if(sceneview){
            let canvas = content.querySelector("canvas")
            renderEngine2D.setDefaultCanvas(canvas)

            let player = this.sceneView.animationPlayer
            huahuoEngine.setActivePlayer(player)

            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(sceneview.storeId)

            let timeline: HHTimeline = document.querySelector("hh-timeline")
            timeline.reloadTracks()
        }
    }

    onNewElement(e:PointerEvent){
        let worldPos = BaseShapeDrawer.getWorldPosFromView(e.x, e.y)

        // Create shape in the original scene/element
        let newElementShape = new ElementShapeJS()
        newElementShape.createShape()
        newElementShape.position = new paper.Point(worldPos.x, worldPos.y)
        newElementShape.store()

        let currentLayer = huahuoEngine.GetCurrentLayer()
        currentLayer.addShape(newElementShape)

        let newStore = huahuoEngine.GetDefaultObjectStoreManager().CreateStore();
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(newStore.GetStoreId())

        let newEleContent = document.createElement("hh-content")
        newEleContent.title = "NewElement"
        newEleContent.style.width = "100%"
        newEleContent.style.height = "100%"
        newEleContent.style.flexBasis = "100%"
        newEleContent.style.alignItems = "stretch"

        let elementSceneView:SceneView = document.createElement("hh-sceneview") as SceneView
        elementSceneView.id = "NewElement"
        elementSceneView.style.flexBasis = "100%"
        elementSceneView.style.display = "flex"
        elementSceneView.style.alignItems = "stretch"

        newEleContent.appendChild(elementSceneView)
        let idx = this.sceneViewPanel.addContent(newEleContent)
        this.sceneViewPanel.selectTab(idx)

        elementSceneView.storeId = newStore.GetStoreId()
        newElementShape.storeId = newStore.GetStoreId()

        console.log("Created new store, store id:" + elementSceneView.storeId)
    }
}

let elementCreator = new ElementCreator()

export {elementCreator}