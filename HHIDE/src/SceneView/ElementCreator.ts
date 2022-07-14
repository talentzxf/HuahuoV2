import {SceneView} from "./SceneView";
import {findParentContent, findParentPanel} from "../Utilities/PanelUtilities";
import {HHPanel} from "hhpanel";
import {renderEngine2D, huahuoEngine, ElementShapeJS, paper} from "hhenginejs"
import {HHContent, PanelEventNames} from "hhpanel";
import {HHTimeline} from "hhtimeline"
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {sceneViewManager} from "./SceneViewManager";

class ElementCreator {
    sceneView: SceneView
    sceneViewPanel: HHPanel

    // If some element is changed, update all related views.
    elementChangeListeners: Map<number, Array<Function>> = new Map()

    constructor() {
        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            _this.sceneView = document.querySelector("#mainScene")
            _this.sceneViewPanel = findParentPanel(this.sceneView)

            let outmostDiv = document.querySelector("#outmost_container")
            outmostDiv.addEventListener(PanelEventNames.CONTENTSELECTED, _this.onContentSelected.bind(_this))
        })
    }

    registerElementChangeEvent(storeId, func: Function){
        if(!this.elementChangeListeners.has(storeId)){
            this.elementChangeListeners.set(storeId, new Array())
        }

        this.elementChangeListeners.get(storeId).push(func)
    }

    dispatchElementChange(storeId){
        let funcArray = this.elementChangeListeners.get(storeId)
        if(funcArray){
            for(let func of funcArray){
                func()
            }
        }
    }

    onContentSelected(e) {
        let content: HHContent = e.detail.content

        let sceneview = content.querySelector("hh-sceneview")

        this.sceneView = sceneview
        this.sceneViewPanel = findParentPanel(this.sceneView)
        if (sceneview) {

            if(sceneViewManager.getFocusedSceneView() == sceneview)
                return;

            sceneViewManager.focusSceneView(sceneview)

            let canvas = content.querySelector("canvas")
            renderEngine2D.setDefaultCanvas(canvas)

            let player = this.sceneView.animationPlayer
            huahuoEngine.setActivePlayer(player)
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(sceneview.storeId)
            player.updateAllShapes()

            let timeline: HHTimeline = document.querySelector("hh-timeline")
            timeline.reloadTracks()

            sceneview.resetDefaultShapeDrawer()
        }
    }

    openElementEditTab(element: ElementShapeJS) {

        let eleSceneView = sceneViewManager.getSceneView(element.storeId)
        if (!eleSceneView) {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(element.storeId)

            let elementId = "NewElement_" + Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);

            let newEleContent = document.createElement("hh-content")
            newEleContent.title = elementId
            newEleContent.style.width = "100%"
            newEleContent.style.height = "100%"
            newEleContent.style.flexBasis = "100%"
            newEleContent.style.alignItems = "stretch"

            let elementSceneView: SceneView = document.createElement("hh-sceneview") as SceneView
            elementSceneView.id = elementId
            elementSceneView.style.flexBasis = "100%"
            elementSceneView.style.display = "flex"
            elementSceneView.style.alignItems = "stretch"

            newEleContent.appendChild(elementSceneView)
            let idx = this.sceneViewPanel.addContent(newEleContent)
            this.sceneViewPanel.selectTab(idx)

            elementSceneView.storeId = element.storeId

            elementSceneView.animationPlayer.loadShapesFromStore()
            elementSceneView.animationPlayer.updateAllShapes()
        }else{ // Switch to the SceneView
            let panel:HHPanel = findParentPanel(eleSceneView)
            let hhcontent:HHContent = findParentContent(eleSceneView)
            let title = hhcontent.getTitle()

            if(panel && title){
                panel.selectTab(title.tabIndex)
            }
        }
    }

    onNewElement(e: PointerEvent) {
        let worldPos = BaseShapeDrawer.getWorldPosFromView(e.x, e.y)

        // Create shape in the original scene/element
        let newElementShape = new ElementShapeJS()
        newElementShape.createShape()
        newElementShape.position = new paper.Point(worldPos.x, worldPos.y)
        newElementShape.store()

        let currentLayer = huahuoEngine.GetCurrentLayer()
        currentLayer.addShape(newElementShape)

        let newStore = huahuoEngine.GetDefaultObjectStoreManager().CreateStore();
        newElementShape.storeId = newStore.GetStoreId()

        this.openElementEditTab(newElementShape)

        console.log("Created new store, store id:" + newElementShape.storeId)

        this.registerElementChangeEvent(newElementShape.storeId, function(){
            newElementShape.update()
        })
    }
}

let elementCreator = new ElementCreator()

export {elementCreator}