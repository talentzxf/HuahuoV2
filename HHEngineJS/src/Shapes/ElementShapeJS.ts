import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {huahuoEngine} from "../EngineAPI";
import {GlobalConfig} from "../GlobalConfig";
import {LayerShapesManager} from "../Player/LayerShapesManager";

let shapeName = "ElementShape"

class ElementShapeJS extends BaseShapeJS {
    static createElement(rawObj) {
        return new ElementShapeJS(rawObj)
    }

    name:string = "unknown"

    emptyPlaceHolder: paper.Group

    size: paper.Point

    layerShapesManager: LayerShapesManager = new LayerShapesManager

    layerFrameMap: Map<any, number> = new Map();

    getName(){
        return this.name
    }

    setName(eleName: string){
        this.name = eleName
    }

    constructor(rawObj) {
        super(rawObj);

        this.size = new paper.Point(100, 100)
    }

    protected isUpdateFillColor(): boolean {
        return false;
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
    }

    get storeId(): number {
        return this.rawObj.GetStoreId();
    }

    set storeId(val: number) {
        this.rawObj.SetStoreId(val)

        huahuoEngine.registerElementParent(val, this.getBornStoreId())
    }

    calculateLocalFrame(){
        let currentFrame = this.getLayer().GetCurrentFrame()
        let bornFrame = this.rawObj.GetBornFrameId()
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

            let currentLocalFrame = this.calculateLocalFrame()
            this.layerShapesManager.forEachLayerInStore(store, (layer)=>{
                this.saveLayerFrame(layer, layer.GetCurrentFrame())

                layer.SetCurrentFrame(currentLocalFrame)
            })

            this.layerShapesManager.loadShapesFromStore(this)

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

            this.layerShapesManager.updateAllShapes()

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
}

shapeFactory.RegisterClass(shapeName, ElementShapeJS.createElement)
export {ElementShapeJS}