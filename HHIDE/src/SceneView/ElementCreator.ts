import {SceneView} from "./SceneView";
import {findParentContent, findParentPanel} from "hhpanel";
import {HHPanel} from "hhpanel";
import {HHToast} from "hhcommoncomponents";
import {huahuoEngine, ElementShapeJS, paper} from "hhenginejs"
import {HHContent, PanelEventNames} from "hhpanel";
import {sceneViewManager} from "./SceneViewManager";
import {BaseShapeJS, Utils} from "hhenginejs";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {elementUploader} from "../RESTApis/ElementUploader";
import {formManager} from "../Utilities/FormManager";
import {UploadElementForm} from "../UIComponents/UploadElementForm";

declare var Module:any;

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

            huahuoEngine.registerEventListener("HHEngine", "BeforeJSShapeCreated", _this.onShapeCreated.bind(_this))

            huahuoEngine.registerEventListener("HHEngine", "onEditElement", _this.editElement.bind(_this))

            huahuoEngine.registerEventListener("HHEngine", "onUploadElement", _this.uploadElement.bind(_this))

            let storeAddedHandler = new Module.ScriptEventHandlerImpl()
            storeAddedHandler.handleEvent = _this.OnRootStoreAdded.bind(_this)

            huahuoEngine.GetInstance().RegisterEvent("OnRootStoreAdded", storeAddedHandler)

        })
    }

    editElement(element){
        this.openElementEditTab(element)
    }

    uploadElement(element){
        let uploadElementForm = formManager.openForm(UploadElementForm)
        uploadElementForm.setStore(element.storeId, element.name)
        uploadElementForm.onOKAction = ()=>{
            elementUploader.uploadStore(element.storeId, element.name)
        }
    }

    onShapeCreated(newShape) {
        if (newShape.getTypeName() == "ElementShape") {
            if (Utils.isValidGUID(newShape.storeId)) {
                this.registerElementChangeEvent(newShape.storeId, function () {
                    newShape.update()
                })
            }
        }

        newShape.registerValueChangeHandler("index")(() => {
            elementCreator.dispatchElementChange(newShape.bornStoreId)
        })
    }

    onTabClosed(e) {
        let content: HHContent = e.detail.content
        let sceneView: SceneView = content.querySelector("hh-sceneview")
        let storeId = sceneView.storeId
        sceneViewManager.removeSceneViewMap(storeId)
    }

    registerElementChangeEvent(storeId, func: Function) {
        if (!this.elementChangeListeners.has(storeId)) {
            this.elementChangeListeners.set(storeId, new Array())
        }

        this.elementChangeListeners.get(storeId).push(func)
    }

    dispatchElementChange(storeId) {
        let targetStoreId = storeId

        while (targetStoreId) {
            this.internalDispatchElementChange(targetStoreId)
            targetStoreId = huahuoEngine.getElementParentByStoreId(targetStoreId)
        }
    }

    internalDispatchElementChange(storeId) {
        // Execute all callback functions
        let funcArray = this.elementChangeListeners.get(storeId)
        if (funcArray) {
            for (let func of funcArray) {
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

            if (sceneViewManager.getFocusedSceneView() == sceneview)
                return;

            sceneViewManager.focusSceneView(sceneview)
        }
    }

    openElementEditTab(element: ElementShapeJS) {

        let eleSceneView = sceneViewManager.getSceneView(element.storeId)
        if (!eleSceneView) {
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

            elementSceneView.setStoreId(element.storeId)

            elementSceneView.animationPlayer.loadShapesFromStore()
            elementSceneView.animationPlayer.updateAllShapes(true)
        } else { // Switch to the SceneView
            let panel: HHPanel = findParentPanel(eleSceneView)
            let hhcontent: HHContent = findParentContent(eleSceneView)
            let title = hhcontent.getTitle()

            if (panel && title) {
                panel.selectTab(title.tabIndex)
            }
        }

        // After element window opened, hide the inspector to avoid confusion.
        IDEEventBus.getInstance().emit(EventNames.UNSELECTOBJECTS)
    }

    onNewElement(openElementTab: boolean = true, storeId: string = null) {

        let elementName = "NewElement_" + Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);

        // Create shape in the original scene/element
        let newElementShape = new ElementShapeJS()
        newElementShape.name = elementName
        newElementShape.createShape()
        newElementShape.position = new paper.Point(0, 0)
        newElementShape.store()

        let currentLayer = huahuoEngine.GetCurrentLayer()
        currentLayer.addShape(newElementShape)

        if(!storeId){
            let newStore = huahuoEngine.GetDefaultObjectStoreManager().CreateStore();
            newElementShape.storeId = newStore.GetStoreId()
        }else{
            newElementShape.storeId = storeId
        }


        if (openElementTab)
            this.openElementEditTab(newElementShape)

        console.log("Created new store, store id:" + newElementShape.storeId)

        this.registerElementChangeEvent(newElementShape.storeId, function () {
            newElementShape.update(true)
        })

        return newElementShape
    }

    createElement(shapes: Set<BaseShapeJS>): ElementShapeJS {
        if(shapes.size == 0)
            return

        let prevStoreId = huahuoEngine.GetCurrentStoreId()

        let allRelatedShapes = new Set<BaseShapeJS>()

        for(let shape of shapes){
            allRelatedShapes.add(shape)
            let referencedShapes = new Set()
            shape.getReferencedShapes(referencedShapes)
            for(let referencedShape of referencedShapes){
                if(referencedShape != null)
                    allRelatedShapes.add(referencedShape)
            }
        }

        if(allRelatedShapes.size > shapes.size){
            if(!window.confirm("Some shapes are referenced, do you want to move them all to the element?")){
                return
            }

            shapes = allRelatedShapes
        }

        try {
            // 0. Ensure all shapes are in the same layer.
            if (shapes.size > 0) {
                let firstShape: BaseShapeJS = shapes.values().next().value as BaseShapeJS;
                let firstLayer = firstShape.getLayer()

                for (let shape of shapes) {
                    if (firstLayer.ptr != shape.getLayer().ptr) {
                        HHToast.warn("Can only group shapes in the same layer!")
                        return null;
                    }
                }
            }

            let newElement = this.onNewElement(false) as ElementShapeJS
            // Create Layer for the store as we won't open it. (If we open it, timeline track will create it.)
            let layer = huahuoEngine.GetCurrentStore().CreateLayer(newElement.name)
            layer.GetTimeLineCellManager().MergeCells(0, huahuoEngine.defaultFrameCount)

            // It's much easier to align element with the global coordinate. Or else we need to set the position for all the keyframes
            // of all the underlying shapes!
            newElement.position = newElement.globalToLocal(new paper.Point(0, 0))

            // StoreId of all the underlying elements should be set to the new element
            for (let shape of shapes) {
                shape.setBornStoreId(newElement.storeId)
            }

            // BornFrame of the element is the min of all underlying shapes.
            let bornFrameId = newElement.bornFrameId

            for (let shape of shapes) {
                if (shape.bornFrameId < bornFrameId)
                    bornFrameId = shape.bornFrameId
            }
            newElement.bornFrameId = bornFrameId

            // Update the position of all the shapes.
            for (let shape of shapes) {
                shape.removePaperObj()
                newElement.addShape(shape)
                shape.addAnimationOffset(-bornFrameId);
                shape.bornFrameId -= bornFrameId
            }

            newElement.syncStoreLayerInfo() // Update the maxFrameId and keyFrameIds in the layer.
            newElement.update()

            HHToast.info(i18n.t("toast.elementCreated", {elementName: newElement.name}))

            huahuoEngine.getActivePlayer().updateAllShapes(true)

            IDEEventBus.getInstance().emit(EventNames.OBJECTSELECTED, newElement.getPropertySheet(), newElement)

            return newElement
        } finally {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStoreId)
        }
    }

    OnRootStoreAdded(args){
        let objectStoreAddedEvent = Module.wrapPointer(args, Module.ObjectStoreAddedEvent)
        let store = objectStoreAddedEvent.GetStore()
        console.log("Root Store added!!!")

        let newElement = this.onNewElement(false, store.GetStoreId())
        HHToast.info(i18n.t("toast.elementLoaded"))
        newElement.update(true)

        // Update the maxFrameId of the current store.
        let maxFrameId = store.GetMaxFrameId();
        huahuoEngine.GetCurrentStore().UpdateMaxFrameId(newElement.bornFrameId + maxFrameId)

        huahuoEngine.getActivePlayer().updateAllShapes(true)
    }
}

let elementCreator = new ElementCreator()

export {elementCreator}