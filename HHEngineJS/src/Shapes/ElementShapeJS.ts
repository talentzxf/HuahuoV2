import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {huahuoEngine} from "../EngineAPI";

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

        let p1 = new paper.Point(0, 0)
        let p2 = this.size
        let boundingBox = new paper.Path.Rectangle(p1, p2)
        boundingBox.strokeColor = new paper.Color("black")
        this.paperItem.addChild(boundingBox)
    }

    get storeId(): number {
        return this.rawObj.GetStoreId();
    }

    set storeId(val: number) {
        this.rawObj.SetStoreId(val)
    }

    updateAllShapes() {
        let defaultStoreManager = huahuoEngine.GetDefaultObjectStoreManager()
        let previousStoreIdx = defaultStoreManager.GetCurrentStore().GetStoreId();
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(this.storeId);
        let store = defaultStoreManager.GetCurrentStore()
        let layerCount = store.GetLayerCount();
        for (let i = 0; i < layerCount; i++) {
            let layer = store.GetLayer(i)

            let shapes = this.getLayerShapes(layer)
            // Try to create layer shapes
            let shapeCount = layer.GetShapeCount()
            for (let shapeId = 0; shapeId < shapeCount; shapeId++) {
                let baseShape = layer.GetShapeAtIndex(shapeId)
                if (!shapes.has(baseShape)) {
                    let shapeConstructor = shapeFactory.GetShapeConstructor(baseShape.GetName())
                    let newBaseShape = shapeConstructor(baseShape)
                    newBaseShape.awakeFromLoad()
                    shapes.set(baseShape, newBaseShape)
                } else { // Update layer shapes
                    let shape = shapes.get(baseShape)
                    shape.update()
                }
            }
        }
        defaultStoreManager.SetDefaultStoreByIndex(previousStoreIdx)
    }

    update(updateOptions = {updateShape: true, updateBoundingBox: true}) {
        if (this.storeId > 0) { // If the storeId is less than 0, the shape has not been inited.
            this.updateAllShapes()
            super.update(updateOptions = {updateShape: true, updateBoundingBox: true})
        }
    }
}

shapeFactory.RegisterClass(shapeName, ElementShapeJS.createElement)
export {ElementShapeJS}