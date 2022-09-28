import {SceneView} from "./SceneView";
import {findParentContent, findParentPanel} from "hhpanel";
import {HHPanel} from "hhpanel";
import {HHToast} from "hhcommoncomponents";
import {renderEngine2D, huahuoEngine, ElementShapeJS, paper} from "hhenginejs"
import {HHContent, PanelEventNames} from "hhpanel";
import {sceneViewManager} from "./SceneViewManager";
import {BaseShapeJS} from "hhenginejs";

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
            outmostDiv.addEventListener(PanelEventNames.TABCLOSED, _this.onTabClosed.bind(_this))

            huahuoEngine.registerEventListener("OnJSShapeCreated", _this.onShapeCreated.bind(_this))
        })
    }

    onShapeCreated(newShape){
        if(newShape.getTypeName() == "ElementShape"){
            this.registerElementChangeEvent(newShape.storeId, function(){
                newShape.update()
            })
        }
    }

    onTabClosed(e){
        let content:HHContent = e.detail.content
        let sceneView:SceneView = content.querySelector("hh-sceneview")
        let storeId = sceneView.storeId
        sceneViewManager.removeSceneViewMap(storeId)
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

            let canvas = content.querySelector(".SceneViewCanvas")
            renderEngine2D.setDefaultCanvas(canvas)

            let player = this.sceneView.animationPlayer
            huahuoEngine.setActivePlayer(player)
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(sceneview.storeId)
            player.updateAllShapes()

            sceneview.resetDefaultShapeDrawer()
        }
    }

    openElementEditTab(element: ElementShapeJS) {

        let eleSceneView = sceneViewManager.getSceneView(element.storeId)
        if (!eleSceneView) {
            console.log("Setting default store by index 2:" + element.storeId)
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(element.storeId)

            let newEleContent = document.createElement("hh-content")
            newEleContent.title = element.name
            newEleContent.style.width = "100%"
            newEleContent.style.height = "100%"
            newEleContent.style.flexBasis = "100%"
            newEleContent.style.alignItems = "stretch"

            let elementSceneView: SceneView = document.createElement("hh-sceneview") as SceneView
            elementSceneView.id = element.name
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

    onNewElement(openElementTab: boolean = true) {

        let elementId = "NewElement_" + Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);

        // Create shape in the original scene/element
        let newElementShape = new ElementShapeJS()
        newElementShape.name = elementId
        newElementShape.createShape()
        newElementShape.position = new paper.Point(0,0)
        newElementShape.store()

        let currentLayer = huahuoEngine.GetCurrentLayer()
        currentLayer.addShape(newElementShape)

        let newStore = huahuoEngine.GetDefaultObjectStoreManager().CreateStore();
        newElementShape.storeId = newStore.GetStoreId()

        if(openElementTab)
            this.openElementEditTab(newElementShape)

        console.log("Created new store, store id:" + newElementShape.storeId)

        this.registerElementChangeEvent(newElementShape.storeId, function(){
            newElementShape.update()
        })

        return newElementShape
    }

    createElement(shapes:Set<BaseShapeJS>): boolean{
        let prevStoreId = huahuoEngine.GetCurrentStoreId()

        try{
            // 0. Ensure all shapes are in the same layer.
            if(shapes.size > 0){
                let firstShape: BaseShapeJS = shapes.values().next().value as BaseShapeJS;
                let firstLayer = firstShape.getLayer()

                for(let shape of shapes){
                    if(firstLayer.ptr != shape.getLayer().ptr)
                    {
                        HHToast.warn("Can only group shapes in the same layer!")
                        return null;
                    }
                }
            }

            let newElement = this.onNewElement(false)
            // Create Layer for the store as we won't open it. (If we open it, timeline track will create it.)
            huahuoEngine.GetCurrentStore().CreateLayer(newElement.name)

            let bornFrameId = newElement.bornFrameId
            for(let shape of shapes){
                if(shape.bornFrameId < bornFrameId)
                    bornFrameId = shape.bornFrameId

                shape.removePaperObj()

                newElement.addShape(shape)
            }

            newElement.update()

            newElement.bornFrameId = bornFrameId

            return newElement
        }finally {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStoreId)
        }
    }
}

let elementCreator = new ElementCreator()

export {elementCreator}