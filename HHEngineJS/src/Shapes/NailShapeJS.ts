import {BaseShapeJS} from "./BaseShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {BaseSolidShape} from "./BaseSolidShape";

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

        // let _this = this
        // // Only register for affine transforms
        // targetShape.registerValueChangeHandler("position|scaling|rotation")(() => {
        //     _this.shapeMoved(targetShape)
        //
        //     for (let [shape, localPoint] of this.shapeLocalPointMap) {
        //         if (shape != targetShape) // No need to update the same shape again.
        //         {
        //             shape.update(true)
        //         }
        //     }
        // })

        return true
    }


    // shapeMoved(shape: BaseSolidShape) {
    //     let localPoint = this.shapeLocalPointMap.get(shape)
    //     if (localPoint == null) {
    //         Logger.error("Why local point is null???")
    //         return
    //     }
    //
    //     this.position = shape.localToGlobal(localPoint)
    //     this.update()
    // }


    createShape() {
        super.createShape();
        this.paperShape = new paper.Path.RegularPolygon(this.position, 6, 20);
        this.paperShape.fillColor = new paper.Color(0.71, 0.25, 0.4, 1.0)
        this.paperShape.strokeColor = new paper.Color("black")
        this.paperShape.data.meta = this
        this.paperShape.bringToFront()

        super.afterCreateShape()
    }
}

clzObjectFactory.RegisterClass(shapeName, NailShapeJS.createNail)
export {NailShapeJS}