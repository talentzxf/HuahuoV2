import {BaseShapeJS} from "./BaseShapeJS";
import {huahuoEngine} from "../EngineAPI";
import {LayerShapesManager} from "../Player/LayerShapesManager";
import {clzObjectFactory} from "../CppClassObjectFactory";

let shapeName = "ElementShape"

class ElementShapeJS extends BaseShapeJS {
    static createElement(rawObj) {
        return new ElementShapeJS(rawObj)
    }

    emptyPlaceHolder: paper.Group

    size: paper.Point

    layerShapesManager: LayerShapesManager

    layerFrameMap: Map<any, number> = new Map();

    constructor(rawObj) {
        super(rawObj);

        this.size = new paper.Point(100, 100)
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

        let pointText = new paper.PointText(new paper.Point(0,0))
        pointText.content = this.name

        this.emptyPlaceHolder = new paper.Group()
        this.emptyPlaceHolder.addChild(boundingBox)
        this.emptyPlaceHolder.addChild(pointText)
        this.paperItem.addChild(this.emptyPlaceHolder)

        super.afterCreateShape()
    }

    get storeId(): number {
        return this.rawObj.GetElementStoreId();
    }

    set storeId(val: number) {
        this.rawObj.SetElementStoreId(val)

        huahuoEngine.registerElementParent(val, this.getBornStoreId())
    }

    // Not sure why, but if we don't write this getter/setter, it will fail??
    get bornFrameId(){
        return this.rawObj.GetBornFrameId()
    }

    set bornFrameId(val:number){
        this.rawObj.SetBornFrameId(val)
    }

    calculateLocalFrame(){
        let currentFrame = this.getLayer().GetCurrentFrame()
        let bornFrame = this.bornFrameId
        let maxFrames = huahuoEngine.getStoreMaxFrames(this.storeId)

        return (currentFrame - bornFrame) % maxFrames
    }

    saveLayerFrame(layer, frame){
        this.layerFrameMap.set(layer, frame)
    }

    restoreLayerFrameIds(){
        let layers = this.layerFrameMap.keys()
        for(let layer of layers){
            let previousFrame = this.layerFrameMap.get(layer)
            layer.SetCurrentFrame(previousFrame)
        }
    }

    override duringUpdate() {
        super.duringUpdate()

        let defaultStoreManager = huahuoEngine.GetDefaultObjectStoreManager()
        let previousStoreIdx = defaultStoreManager.GetCurrentStore().GetStoreId();

        try{
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId);

            let store = defaultStoreManager.GetCurrentStore()

            if(this.layerShapesManager == null){
                this.layerShapesManager = new LayerShapesManager(this.storeId)
            }

            let currentLocalFrame = this.calculateLocalFrame()
            this.layerShapesManager.loadShapesFromStore(this)

            this.layerShapesManager.forEachLayerInStore(store, (layer)=>{
                this.saveLayerFrame(layer, layer.GetCurrentFrame())

                layer.SetCurrentFrame(currentLocalFrame)
            })

            let somethingIsVisible = false
            let _this = this
            this.layerShapesManager.forEachShapeInStore(store, (shape)=>{
                if(shape){
                    _this.emptyPlaceHolder.remove()
                    if(shape.isVisible()){
                        somethingIsVisible = true
                    }
                }
            })

            if(!somethingIsVisible)
                this.selected = false

            if(this.isVisible()){
                this.layerShapesManager.updateAllShapes()
            } else{
                this.layerShapesManager.hideAllShapes() // This element is invisible, hide all it's containing shapes.
            }
        }finally {
            this.restoreLayerFrameIds();
            defaultStoreManager.SetDefaultStoreByIndex(previousStoreIdx)
        }
    }

    update() {
        if (this.storeId > 0) { // If the storeId is less than 0, the shape has not been inited.
            super.update()
        }
    }

    awakeFromLoad() {
        super.awakeFromLoad();
        huahuoEngine.RegisterElementShape(this.storeId, this);
    }

    addShape(shape: BaseShapeJS){
        let prevStoreId = huahuoEngine.GetCurrentStoreId()
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId)

        try{
            // 1. Remove the shape from current layer (both cpp and js side)
            shape.detachFromCurrentLayer();

            // After detach, default store might be changed.
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId)

            // 2. Add the shape into current layer of this store. And it will be loaded by the element.
            huahuoEngine.GetCurrentLayer().AddShapeInternal(shape.getRawShape())
        }finally {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStoreId)
        }
    }

    syncStoreLayerInfo(){
        huahuoEngine.GetDefaultObjectStoreManager().GetStoreById(this.storeId).SyncLayersInfo()
    }
}

clzObjectFactory.RegisterClass(shapeName, ElementShapeJS.createElement)
export {ElementShapeJS}