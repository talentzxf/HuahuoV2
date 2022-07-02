import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {huahuoEngine} from "../EngineAPI";

let shapeName = "ElementShape"
class ElementShapeJS extends BaseShapeJS{
    static createElement(rawObj){
        return new ElementShapeJS(rawObj)
    }

    size: paper.Point

    createdLayers: Set<any> = new Set()

    constructor(rawObj) {
        super(rawObj);

        this.size = new paper.Point(100, 100)
    }
    getShapeName(): string {
        return shapeName
    }

    // Render a box first.
    createShape() {
        let paper = this.getPaperJs()
        this.paperItem = new paper.Group()
        this.paperItem.applyMatrix = false
        this.paperItem.data.meta = this

        let p1 = new paper.Point(0,0)
        let p2 = this.size
        let boundingBox = new paper.Path.Rectangle(p1, p2)
        boundingBox.strokeColor = new paper.Color("black")
        this.paperItem.addChild(boundingBox)
    }

    get storeId():number{
        return this.rawObj.GetStoreId();
    }

    set storeId(val:number){
        this.rawObj.SetStoreId(val)
    }

    updateAllShapes(){
        let defaultStoreManager = huahuoEngine.GetDefaultObjectStoreManager()
        let previousStoreIdx = defaultStoreManager.GetCurrentStore().GetStoreId();
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId);
        let store = defaultStoreManager.GetCurrentStore()
        let layerCount = store.GetLayerCount();
        for(let i = 0 ; i < layerCount; i++){
            let layer = store.GetLayer(i)
            huahuoEngine.updateLayerShapes(layer)
        }
        defaultStoreManager.SetDefaultStoreByIndex(previousStoreIdx)
    }

    update(updateOptions = {updateShape: true, updateBoundingBox: true}){
        if(this.storeId > 0){ // If the storeId is less than 0, the shape has not been inited.
            this.updateAllShapes()
            super.update(updateOptions = {updateShape: true, updateBoundingBox: true})
        }
    }
}

shapeFactory.RegisterClass(shapeName, ElementShapeJS.createElement)
export {ElementShapeJS}