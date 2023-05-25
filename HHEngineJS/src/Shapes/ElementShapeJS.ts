import {BaseShapeJS} from "./BaseShapeJS";
import {huahuoEngine} from "../EngineAPI";
import {LayerShapesManager} from "../Player/LayerShapesManager";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {PropertyType} from "hhcommoncomponents";
import {ElementController} from "../Components/ElementController";
import {Utils} from "./Utils";

let shapeName = "ElementShape"

let elementCreated: number = 0
let elementUpdated: number = 0

class ElementShapeJS extends BaseShapeJS {
    static createElement(rawObj) {
        return new ElementShapeJS(rawObj)
    }

    emptyPlaceHolder: paper.Group

    size: paper.Point

    layerShapesManager: LayerShapesManager

    layerFrameMap: Map<any, number> = new Map();

    constructor(rawObj) {
        let needAddComponent = !rawObj
        super(rawObj);

        if (needAddComponent) {
            this.addComponent(new ElementController())
        }

        this.size = new paper.Point(100, 100)

        elementCreated++

        console.log("ELement created count:" + elementCreated)
    }

    protected isUpdateStrokeColor(): boolean {
        return false;
    }

    getShapeName(): string {
        return shapeName
    }

    // Render a box first.
    createShape() {
        super.createShape()

        let paper = this.getPaperJs()
        this.paperItem = new paper.Group()
        this.paperItem.applyMatrix = false
        this.paperItem.data.meta = this

        // Draw a box to indicate this is an element
        let p1 = new paper.Point(0, 0)
        let p2 = this.size
        let boundingBox = new paper.Path.Rectangle(p1, p2)
        boundingBox.fillColor = new paper.Color("gray")
        boundingBox.fillColor.alpha = 0.5
        boundingBox.strokeColor = new paper.Color("black")
        boundingBox.data.meta = this

        let pointText = new paper.PointText(new paper.Point(0, 0))
        pointText.content = this.name

        this.emptyPlaceHolder = new paper.Group()
        this.emptyPlaceHolder.addChild(boundingBox)
        this.emptyPlaceHolder.addChild(pointText)
        this.paperItem.addChild(this.emptyPlaceHolder)

        super.afterCreateShape()
    }

    get storeId(): string {
        return this.rawObj.GetElementStoreId();
    }

    set storeId(val: string) {
        this.rawObj.SetElementStoreId(val)

        huahuoEngine.registerElementParent(val, this.getBornStoreId())
    }

    // Not sure why, but if we don't write this getter/setter, it will fail??
    get bornFrameId() {
        return this.rawObj.GetBornFrameId()
    }

    set bornFrameId(val: number) {
        this.rawObj.SetBornFrameId(val)
    }

    calculateLocalFrame() {
        let currentFrame = this.getLayer().GetCurrentFrame()
        let bornFrame = this.bornFrameId
        let maxFrames = huahuoEngine.getStoreMaxFrames(this.storeId)

        if(maxFrames == 0){
            return 0;
        }

        return (((currentFrame - bornFrame) * this.getPlaySpeed()) % maxFrames + maxFrames) % maxFrames
    }

    saveLayerFrame(layer, frame) {
        this.layerFrameMap.set(layer, frame)
    }

    restoreLayerFrameIds() {
        let layers = this.layerFrameMap.keys()
        for (let layer of layers) {
            let previousFrame = this.layerFrameMap.get(layer)
            layer.SetCurrentFrame(previousFrame)
        }
    }

    override preparePaperItem(force: boolean = false) {
        super.preparePaperItem(force)

        let defaultStoreManager = huahuoEngine.GetDefaultObjectStoreManager()
        let previousStoreIdx = defaultStoreManager.GetCurrentStore().GetStoreId();

        try {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId);

            let store = defaultStoreManager.GetCurrentStore()

            if (this.layerShapesManager == null) {
                this.layerShapesManager = new LayerShapesManager(this.storeId)
            }

            let currentLocalFrame = this.calculateLocalFrame()
            this.layerShapesManager.forEachLayerInStore(store, (layer) => {
                this.saveLayerFrame(layer, layer.GetCurrentFrame())

                layer.SetCurrentFrame(currentLocalFrame)
            })

            this.layerShapesManager.loadShapesFromStore(this)

            let somethingIsVisible = false
            let _this = this
            this.layerShapesManager.forEachShapeInStore(store, (shape) => {
                if (shape) {
                    _this.emptyPlaceHolder.remove()
                    if (shape.isVisible()) {
                        somethingIsVisible = true
                    }
                }
            })

            if (!somethingIsVisible)
                this.selected = false

            if (this.isVisible()) {
                this.layerShapesManager.updateAllShapes(force)
            } else {
                this.layerShapesManager.hideAllShapes() // This element is invisible, hide all it's containing shapes.
            }
        } catch (e) {
            console.error(e)
        } finally {
            this.restoreLayerFrameIds();
            defaultStoreManager.SetDefaultStoreByIndex(previousStoreIdx)
        }
    }



    update(force: boolean = false) {
        if (Utils.isValidGUID(this.storeId)) { // If the storeId is less than 0, the shape has not been inited.
            super.update(force)
            elementUpdated++
        }
    }

    awakeFromLoad() {
        super.awakeFromLoad();
        huahuoEngine.RegisterElementShape(this.storeId, this);
    }

    addShape(shape: BaseShapeJS) {
        let prevStoreId = huahuoEngine.GetCurrentStoreId()
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId)

        try {
            // 1. Remove the shape from current layer (both cpp and js side)
            shape.detachFromCurrentLayer();

            // After detach, default store might be changed.
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId)

            // 2. Add the shape into current layer of this store. And it will be loaded by the element.
            huahuoEngine.GetCurrentLayer().AddShapeInternal(shape.getRawShape())
        } finally {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStoreId)
        }
    }

    syncStoreLayerInfo() {
        huahuoEngine.GetDefaultObjectStoreManager().GetStoreById(this.storeId).SyncLayersInfo()
    }

    // TODO: These functions should only exist in HHIDE, should move to IDE.
    onEditElement() {
        huahuoEngine.dispatchEvent("HHEngine", "onEditElement", this)
    }

    onUploadElement(){
        huahuoEngine.dispatchEvent("HHEngine", "onUploadElement", this)
    }

    _elementController = null

    get elementController() {
        if (this._elementController == null)
            this._elementController = this.getComponentByTypeName("ElementController")
        return this._elementController
    }

    getPlaySpeed() {
        return this.elementController.playSpeed
    }



    additionalPropertyAdded: boolean = false
    getPropertySheet(){
        if(!this.additionalPropertyAdded){
            this.additionalPropertyAdded = true
            this.propertySheet.addProperty({
                key: "inspector.editElement",
                type: PropertyType.BUTTON,
                config: {
                    action: this.onEditElement.bind(this)
                }
            })

            this.propertySheet.addProperty({
                key: "inspector.uploadElement",
                type: PropertyType.BUTTON,
                config:{
                    action: this.onUploadElement.bind(this)
                }
            })
        }

        return super.getPropertySheet()
    }
}

clzObjectFactory.RegisterClass(shapeName, ElementShapeJS.createElement)
export {ElementShapeJS}