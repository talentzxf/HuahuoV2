import {BaseShapeJS} from "./BaseShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {BaseSolidShape} from "./BaseSolidShape";
import {getNailManager} from "../IK/NailManager";
import {huahuoEngine} from "../EngineAPI";

class ShapeArrayIterable {
    nailCppObject

    constructor(nailCppObject) {
        this.nailCppObject = nailCppObject
    }

    [Symbol.iterator]() {
        let curIdx = 0
        let _this = this
        const iterator = {
            next(){
                if(curIdx < _this.nailCppObject.GetShapeCount()){
                    let baseShapeRawObj = _this.nailCppObject.GetShapeAtIndex(curIdx++)
                    let baseShapeJS = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(baseShapeRawObj)
                    return {value: baseShapeJS, done: false}
                }

                return {value: null, done: true}
            }
        }

        return iterator
    }
}

let shapeName = "NailShape"
class NailShapeJS extends BaseShapeJS{
    static createNail(rawObj){
        return new NailShapeJS(rawObj)
    }

    getShapeName(): string {
        return shapeName
    }

    // The point is in global world space.
    addShape(targetShape: BaseSolidShape, globalPosition: paper.Point){
        let localPoint = targetShape.globalToLocal(globalPosition)
        this.rawObj.AddShape(targetShape.getRawShape(), localPoint.x, localPoint.y, 0.0)
        this.rawObj.SetGlobalPivotPosition(globalPosition.x, globalPosition.y, 0.0)

        let _this = this
        // Only register for affine transforms
        targetShape.registerValueChangeHandler("position|scaling|rotation")(() => {
            getNailManager().shapeMoved(targetShape)
        })
    }

    createShape() {
        super.createShape();
        this.paperShape = new paper.Path.RegularPolygon(this.position, 6, 20);
        this.paperShape.fillColor = new paper.Color(0.71, 0.25, 0.4, 1.0)
        this.paperShape.strokeColor = new paper.Color("black")
        this.paperShape.data.meta = this
        this.paperShape.applyMatrix = false
        this.paperShape.bringToFront()

        super.afterCreateShape()
    }

    getLocalPositionInShape(targetShape: BaseShapeJS): paper.Point{
        let pos = this.rawObj.GetLocalPositionInShape(targetShape.getRawShape())
        return new paper.Point(pos.x, pos.y)
    }

    getShapes(){
        return new ShapeArrayIterable(this.rawObj)
    }
}

clzObjectFactory.RegisterClass(shapeName, NailShapeJS.createNail)
export {NailShapeJS}