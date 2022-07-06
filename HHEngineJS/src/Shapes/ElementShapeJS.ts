import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {huahuoEngine} from "../EngineAPI";
import {GlobalConfig} from "../GlobalConfig";

let shapeName = "ElementShape"

class ElementShapeJS extends BaseShapeJS {
    static createElement(rawObj) {
        return new ElementShapeJS(rawObj)
    }

    size: paper.Point

    layerShapes: Map<any, Map<any, BaseShapeJS>> = new Map();

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

    getLayerShapes(layer): Map<any, BaseShapeJS> {

        if (!this.layerShapes.has(layer)) {
            this.layerShapes.set(layer, new Map())
        }

        return this.layerShapes.get(layer)
    }

    // Render a box first.
    createShape() {
        let paper = this.getPaperJs()
        this.paperItem = new paper.Group()
        this.paperItem.applyMatrix = false
        this.paperItem.data.meta = this

        // // Draw a box to indicate this is an element
        // let p1 = new paper.Point(0, 0)
        // let p2 = this.size
        // let boundingBox = new paper.Path.Rectangle(p1, p2)
        // boundingBox.strokeColor = new paper.Color("black")
        // this.paperItem.addChild(boundingBox)
    }

    get storeId(): number {
        return this.rawObj.GetStoreId();
    }

    set storeId(val: number) {
        this.rawObj.SetStoreId(val)
    }

    calculateLocalFrame(){
        let currentFrame = this.getLayer().GetCurrentFrame()
        let bornFrame = this.rawObj.GetBornFrameId()

        return currentFrame - bornFrame
    }

    updateAllShapes() {
        let defaultStoreManager = huahuoEngine.GetDefaultObjectStoreManager()
        let previousStoreIdx = defaultStoreManager.GetCurrentStore().GetStoreId();
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId);
        let store = defaultStoreManager.GetCurrentStore()
        let layerCount = store.GetLayerCount();

        let currentLocalFrame = this.calculateLocalFrame()
        let somethingIsVisible = false
        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)

            layer.SetCurrentFrame(currentLocalFrame)

            let shapes = this.getLayerShapes(layer)
            // Try to create layer shapes
            let shapeCount = layer.GetShapeCount()
            for (let shapeId = 0; shapeId < shapeCount; shapeId++) {
                let baseShape = layer.GetShapeAtIndex(shapeId)

                let shape = null
                if (!shapes.has(baseShape)) {
                    let shapeConstructor = shapeFactory.GetShapeConstructor(baseShape.GetName())
                    let newBaseShape = shapeConstructor(baseShape)
                    newBaseShape.awakeFromLoad()
                    shapes.set(baseShape, newBaseShape)
                    newBaseShape.setParent(this)

                    shape = newBaseShape
                } else { // Update layer shapes
                    shape = shapes.get(baseShape)
                }

                shape.update()

                if(shape.isVisible()){
                    somethingIsVisible = true
                }
            }
        }
        defaultStoreManager.SetDefaultStoreByIndex(previousStoreIdx)

        if(!somethingIsVisible)
            this.selected = false
    }

    update() {
        if (this.storeId > 0) { // If the storeId is less than 0, the shape has not been inited.
            this.updateAllShapes()
            super.update()
        }
    }
}

shapeFactory.RegisterClass(shapeName, ElementShapeJS.createElement)
export {ElementShapeJS}